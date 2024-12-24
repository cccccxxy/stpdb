#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <map>
#include <assert.h>
#include <leveldb/db.h>
#include <sstream>
#include <vector>
#include <cctype>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <nlohmann/json.hpp>
#include <chrono>
#include <mutex>
#include <unordered_set>
#include <regex>
#include <filesystem>
#include <queue>

#include <grpcpp/grpcpp.h>
#include "lsmdb.grpc.pb.h"

using namespace std;
using json = nlohmann::json;
std::mutex dbjson_delete_mutex;
std::mutex part_entity_mutex;
std::mutex context_entity_mutex;
int padding = 16;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using lsmdb::v1::CloseDBReply;
using lsmdb::v1::CloseDBRequest;
using lsmdb::v1::GetReply;
using lsmdb::v1::GetRequest;
using lsmdb::v1::Lsmdb;
using lsmdb::v1::PutReply;
using lsmdb::v1::PutRequest;
using lsmdb::v1::BatchPutReply;
using lsmdb::v1::BatchPutRequest;
using lsmdb::v1::PrefixRequest;
using lsmdb::v1::PrefixResponse;
using lsmdb::v1::KeyValue;

class LsmdbClient
{
public:
    explicit LsmdbClient(const std::shared_ptr<grpc::Channel>& channel)
        : stub_(Lsmdb::NewStub(channel)) {}

    bool Put(const std::string &key, const std::string &value)
    {
        PutRequest request;
        request.set_key(key);
        request.set_value(value);

        PutReply reply;
        ClientContext context;

        Status status = stub_->Put(&context, request, &reply);
        if (status.ok())
        {
            return reply.data();
        }
        else
        {
            std::cerr << "写入失败" << status.error_message() << std::endl;
            return false;
        }
    }

    bool BatchPut(const vector<std::string> keys, vector<std::string> values)
    {
        BatchPutRequest request;
        for (const auto& key : keys) {
            request.add_keys(key);
        }
        for (const auto& value : values) {
            request.add_values(value);
        }

        BatchPutReply reply;
        ClientContext context;

        Status status = stub_->BatchPut(&context, request, &reply);
        if (status.ok())
        {
            return reply.data();
        }
        else
        {
            std::cerr << "写入失败" << status.error_message() << std::endl;
            return false;
        }
    }

    std::string Get(const std::string &key)
    {
        GetRequest request;
        GetReply reply;
        ClientContext context;
        request.set_key(key);

        Status status = stub_->Get(&context, request, &reply);
        if (status.ok())
        {
            return reply.value();
        }
        else
        {
            std::cerr << "读取失败" << status.error_message() << std::endl;
            return "";
        }
    }

    bool CloseDB()
    {
        CloseDBRequest request;
        CloseDBReply reply;
        ClientContext context;

        Status status = stub_->CloseDB(&context, request, &reply);
        if (status.ok())
        {
            std::cout << "数据库关闭成功" << std::endl;
            return true;
        }
        else
        {
            std::cerr << "关闭失败" << status.error_message() << std::endl;
            return false;
        }
    }
    vector<pair<string, string>> GetDataWithPrefix(const std::string &prefixKey)
    { 
        PrefixRequest request;
        PrefixResponse reply;
        ClientContext context;
        request.set_prefixkey(prefixKey);

        Status status = stub_->PrefixData(&context, request, &reply);

        if (status.ok()) {
            vector<pair<string, string>> result;
            // 遍历返回的键值对，并将它们添加到结果向量中
            for (int i = 0; i < reply.keyvaluelist_size(); ++i) {
                const KeyValue& kv = reply.keyvaluelist(i);
                result.emplace_back(kv.key(), kv.value());
            }
            return result;
        } else {
            std::cerr << "GetDataWithPrefix 调用失败: " << status.error_message() << endl;
            return {};
        }
    }

private:
    std::unique_ptr<Lsmdb::Stub> stub_;
};

std::shared_ptr<grpc::Channel> CreateChannelWithMaxMessageSize(const std::string& server_address) {
    grpc::ChannelArguments channel_args;
    channel_args.SetMaxReceiveMessageSize(1024 * 1024 * 1024); // 设置为 100MB
    channel_args.SetMaxSendMessageSize(1024 * 1024 * 1024); // 设置为 100MB

    return grpc::CreateCustomChannel(
        server_address,
        grpc::InsecureChannelCredentials(), // 使用不安全的信道凭证
        channel_args // 传入 ChannelArguments
    );
}

// 读取 STEP 文件，将文件内容转换为字符串
string readStepFile(const string& filepath){
  ifstream file(filepath);
  if (!file.is_open())
    throw runtime_error("无法打开STEP文件：" + filepath);

  std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  file.close();
  return content;
}

// 自定义函数，判断是否是需要去除的字符
bool isUnwantedChar(unsigned char c) {
    return isspace(c) || c == '#' || c == '\n';
}

// 去除字符串前后多余的字符
string trim(const string& str) {
    // 找到第一个非不需要字符
    auto start = std::find_if_not(str.begin(), str.end(), isUnwantedChar);

    // 找到最后一个非不需要字符
    auto end = std::find_if_not(str.rbegin(), str.rend(), isUnwantedChar).base();

    // 如果全是无用字符，返回空字符串；否则返回截取后的字符串
    return (start < end) ? std::string(start, end) : std::string();
}

// 删除列表对象前后的括号
string trim_brackets(string& list) {
    if (list.size() <= 1) return "";
    return list.substr(1, list.size() - 2);
}

// 将字符串转换为固定长度的字符串，左侧填充 '0'
string leftPad(string str, size_t length){
  if (str.length() >= length){
    return str.substr(0, length);
  }
  return string(length - str.length(), '0') + str;
}

// 将 STEP 文件内容拆分为 key-value 键值对
vector<pair<string, string>> parseStepEntity(string stepContent){
  cout << "parseStepEntity" << endl;
  string entity;
  stringstream inputStep(stepContent);
  vector<pair<string, string>> parseEntity;

  // 以 ";" 为分隔符拆分内容
  while(getline(inputStep, entity, ';')){
    if (!entity.empty()){
      size_t pos = entity.find('=');
      if (pos == string::npos){
        continue; 
      }
      
      string id = trim(entity.substr(0, pos)); // 提取 ID
      //  对 key 进行填充
      id = leftPad(id, 16);
      string value = trim(entity.substr(pos + 1));
      parseEntity.push_back({id, value});
    }
  }
  cout << "parseStepEntity end" << endl;
  return parseEntity;
}

// 将 levelDB 中的 value 值拆分为 entity_type  与 property
void parse_content(string entity_content, string &entity_name, string &property){
  size_t pos = entity_content.find('(');

  if (pos != std::string::npos) {
    entity_name = trim(entity_content.substr(0, pos)); // entity_name 为 '(' 前的内容
    property = trim(entity_content.substr(pos + 1, entity_content.size() - pos - 2)); // property 为 '(' 后 ')' 前的内容
  } 
}

std::vector<std::string> split_parser(const std::string& input) {
    std::vector<std::string> result;
    std::string current;
    int parenthesesCount = 0;

    for (char ch:input) {
        if (ch == '#' || std::isspace(ch) || ch < 0 ) continue;
        
        if (ch == ',' && parenthesesCount == 0) {
            // 逗号在括号外面时，分隔当前部分
            result.push_back(current);
            current.clear();
        }
        else {
            if (ch == '(') {
                parenthesesCount++;
            }
            else if (ch == ')') {
                parenthesesCount--;
            }
            current += ch;
        }

    }
    // 添加最后一部分
    if (!current.empty())   result.push_back(current);
    // 去除每一项的前后空格
    for (auto& item : result)   item = item.substr(item.find_first_not_of(" "), item.find_last_not_of(" ") + 1);
    return result;
}


void getPropertyType(LsmdbClient &client, string entity_id,  string &type, string &property, unordered_set<string> &local_compression_entitys){
  // 在处理压缩的对象过程中，需要对属性内的索引 id 也进行填充
  string entity_key = leftPad(entity_id, 16);
  // cout << "======" << entity_id << endl;
  local_compression_entitys.insert(entity_key);
  // cout << entity_key << endl;
  // cout << "===========================================" << endl;
  std::string result = client.Get(entity_key);
  // leveldb::Status status = client->Get(leveldb::ReadOptions(), entity_key, &content);
  if (result.empty()) {
    // 键不存在，跳过处理
    // 你可以在这里添加任何其他逻辑，比如日志记录
    std::cout << "Key not found: " << entity_key << std::endl;
    
  } else {
    // 键存在，处理内容
    parse_content(result, type, property);
  }

}

//  entity -> vertex -> point ==> entity.point
string cartesian_point_compression(string vertex_id, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
  string vertex_type, vertex_property;
  getPropertyType(client, vertex_id, vertex_type, vertex_property, local_compression_entitys);

  vector<string> matches_vertex = split_parser(vertex_property);

  string cartesian_point_id = matches_vertex[1];
  string point_type, point_property;
  getPropertyType(client, cartesian_point_id, point_type, point_property, local_compression_entitys);

  vector<string> matches_cartesian = split_parser(point_property);
  return matches_cartesian[1];
} 


unordered_map<string, function<json (string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys)>> analysis;


void setupAnalysis(){
  analysis["MANIFOLD_SOLID_BREP"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "MANIFOLD_SOLID_BREP";
    entity["name"] = matches[0];
    entity["outer_id"] = '[' + matches[1] + ']';
    entity["related_entity"] =  matches[1];

    entity["belong"] = "";

    return entity;
  };

  analysis["BREP_WITH_VOIDS"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);
    string related_entity = "";

    json entity;
    entity["entity_type"] = "BREP_WITH_VOIDS";
    entity["name"] = matches[0];
    entity["outer_id"] = '[' + matches[1] + ']';
    entity["voids"] = '[' +  trim_brackets(matches[2])  +']';

    related_entity = related_entity + matches[1];
    if (matches[2] != "")
      related_entity = related_entity + ',' + trim_brackets(matches[2]);

    entity["related_entity"] =  related_entity;

    entity["belong"] = "";

    return entity;
  };


  analysis["SHELL_BASED_SURFACE_MODEL"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "SHELL_BASED_SURFACE_MODEL";
    entity["name"] = matches[0];
    entity["outer_ids_list"] = '[' + trim_brackets(matches[1]) + ']';
    entity["related_entity"] = trim_brackets(matches[1]);

    entity["belong"] = "";

    return entity;  
  };

  analysis["CLOSED_SHELL"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "CLOSED_SHELL";
    entity["name"] = matches[0];
    entity["ids"] = '[' + trim_brackets(matches[1]) + ']';
    entity["related_entity"] = trim_brackets(matches[1]);

    entity["belong"] = "";

    return entity;  
  };

  analysis["ORIENTED_CLOSED_SHELL"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);
    string related_entity = "";

    json entity;
    entity["entity_type"] = "ORIENTED_CLOSED_SHELL";
    entity["name"] = matches[0];
    entity["ids"] = '[' + trim_brackets(matches[1]) + ']';
    entity["closed_shell_element"] = '[' + matches[2] + ']';
    entity["orientation"] = matches[3];

    if ( trim_brackets(matches[1]) != "" ){
      related_entity = related_entity + trim_brackets(matches[1]) + ',';
    }
    related_entity = related_entity + matches[2];

    entity["related_entity"] = related_entity;

    entity["belong"] = "";
    
    return entity;  
  };

  
  analysis["OPEN_SHELL"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);
    json entity;
    entity["entity_type"] = "OPEN_SHELL";
    entity["name"] = matches[0];
    entity["ids"] = '[' + trim_brackets(matches[1]) + ']';
    entity["related_entity"] = trim_brackets(matches[1]);

    entity["belong"] = "";

    return entity;
  };

  analysis["ADVANCED_FACE"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    string name, face_bound_ids, geometry_face_id, same_sense, face_geometry_type, position_id, radius_1, radius_2;
    vector<string> matches = split_parser(property);
    name = matches[0];
    face_bound_ids = trim_brackets(matches[1]);
    geometry_face_id = matches[2];
    same_sense = matches[3];
    radius_1 = "";
    radius_2 = "";
    string related_entity = "";

    // 需要进行压缩时根据记录的 geometry_face_id 来查找数据库
    string geometry_face_property;
    json entity;
    // entity_type(property) --> entity_type, property
    if (geometry_face_id != "$") {
      getPropertyType(client, geometry_face_id, face_geometry_type, geometry_face_property, local_compression_entitys);

      unordered_set<string> compression_surface_type = { "PLANE", "", "SPHERICAL_SURFACE", "CONICAL_SURFACE", "TOROIDAL_SURFACE" };
      
      if (face_geometry_type == "") {
          face_geometry_type = geometry_face_property.substr(0, geometry_face_property.find('('));
      }
      
      if (compression_surface_type.find(face_geometry_type) != compression_surface_type.end()) {
        // property -> 具体属性值
        vector<string> matches_geometry_face_property = split_parser(geometry_face_property);
        position_id = matches_geometry_face_property[1];

        if (matches_geometry_face_property.size() > 2){
          radius_1 = matches_geometry_face_property[2];
          if (matches_geometry_face_property.size() > 3)
            radius_2 = matches_geometry_face_property[3];
        }
        entity["face_geometry_id"] = "";
      }
      else {
        entity["face_geometry_id"] = '[' + geometry_face_id + ']';
        related_entity = related_entity + geometry_face_id + ','; 
        // 找到第一个匹配的对象
        auto it = std::find(local_compression_entitys.begin(), local_compression_entitys.end(), leftPad(geometry_face_id, padding));
        if (it != local_compression_entitys.end()) {
            local_compression_entitys.erase(it); // 删除第一个匹配的对象
        }
      }
    }
    else{
      entity["face_geometry_id"] = "$";
    }
    
    entity["entity_type"] = "ADVANCED_FACE";
    entity["name"] = name;
    entity["position_id"] = '[' + position_id + ']';
    entity["face_geometry_type"] = face_geometry_type;
    entity["same_sense"] = same_sense;
    entity["radius_1"] = radius_1;
    entity["radius_2"] = radius_2;
    entity["face_bound_ids"] = '[' + face_bound_ids + ']';
    if (position_id != "")
      related_entity = related_entity + position_id + ',';
    related_entity = related_entity + face_bound_ids;
    entity["related_entity"] = related_entity;

    entity["belong"] = "";

    return entity;
  };

  analysis["PLANE"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
      vector<string> matches = split_parser(property);

      json entity;
      entity["entity_type"] = "PLANE";
      entity["name"] = matches[0];
      entity["position_id"] = '[' + matches[1] + ']';

      entity["related_entity"] =  matches[1];

      entity["belong"] = "";

      return entity;
  }; 

  analysis["CYLINDRICAL_SURFACE"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
      vector<string> matches = split_parser(property);

      json entity;
      entity["entity_type"] = "CYLINDRICAL_SURFACE";
      entity["name"] = matches[0];
      entity["position_id"] = '[' + matches[1] + ']';
      entity["radius"] = matches[2];

      entity["related_entity"] =  matches[1];

      entity["belong"] = "";

      return entity;
  }; 

  analysis["SPHERICAL_SURFACE"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
      vector<string> matches = split_parser(property);

      json entity;
      entity["entity_type"] = "SPHERICAL_SURFACE";
      entity["name"] = matches[0];
      entity["position_id"] = '[' + matches[1] + ']';
      entity["radius"] = matches[2];

      entity["related_entity"] =  matches[1];

      entity["belong"] = "";

      return entity;
  }; 

  analysis["CONICAL_SURFACE"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
      vector<string> matches = split_parser(property);

      json entity;
      entity["entity_type"] = "CONICAL_SURFACE";
      entity["name"] = matches[0];
      entity["position_id"] = '[' + matches[1] + ']';
      entity["radius"] = matches[2];
      entity["semi_angle"] = matches[3];

      entity["related_entity"] =  matches[1];

      entity["belong"] = "";

      return entity;
  }; 

  analysis["TOROIDAL_SURFACE"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
      vector<string> matches = split_parser(property);

      json entity;
      entity["entity_type"] = "TOROIDAL_SURFACE";
      entity["name"] = matches[0];
      entity["position_id"] = '[' + matches[1] + ']';
      entity["major_radius"] = matches[2];
      entity["minor_radius"] = matches[3];

      entity["related_entity"] =  matches[1];

      entity["belong"] = "";

      return entity;
  }; 


  analysis["SURFACE_OF_REVOLUTION"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
      vector<string> matches = split_parser(property);
      string related_entity = "";

      json entity;
      entity["entity_type"] = "SURFACE_OF_REVOLUTION";
      entity["name"] = matches[0];
      entity["swept_curve_id"] = '[' + matches[1] + ']';
      entity["axis_position_id"] = '[' + matches[2] + ']';

      related_entity = related_entity + matches[1] + ',';
      related_entity = related_entity + matches[2];

      entity["related_entity"] =  related_entity;

      entity["belong"] = "";

      return entity;
  }; 

  analysis["SURFACE_OF_LINEAR_EXTRUSION"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    string name, swept_curve_id, vector_magnitude, vector_orientation;
    vector<string> matches = split_parser(property);
    name = matches[0];
    swept_curve_id = matches[1];

    // 将 vector_node 相关属性进行压缩
    string vector_type, vector_property;
    getPropertyType(client, matches[2], vector_type, vector_property, local_compression_entitys);

    vector<string> matches_vector = split_parser(vector_property);
    vector_magnitude = matches_vector[2];

    string direction_type, direction_property;
    getPropertyType(client, matches_vector[1], direction_type, direction_property, local_compression_entitys);

    vector_orientation = split_parser(direction_property)[1];

    json entity;
    entity["entity_type"] = "SURFACE_OF_LINEAR_EXTRUSION";
    entity["name"] = name;
    entity["swept_curve_id"] = '[' + swept_curve_id + ']';
    entity["vector_magnitude"] = vector_magnitude;
    entity["vector_orientation"] = vector_orientation;

    entity["related_entity"] = swept_curve_id;

    entity["belong"] = "";

    return entity;
  };

  analysis["OFFSET_SURFACE"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "OFFSET_SURFACE";
    entity["name"] = matches[0];
    entity["surface_id"] = '[' + matches[1] + ']';
    entity["distance"] = matches[2];
    entity["self_intersect"] = matches[3];

    entity["related_entity"] = matches[1];

    entity["belong"] = "";

    return entity;
  };

  analysis["B_SPLINE_CURVE_WITH_KNOTS"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    string name, degree, control_points_list, control_points_list_id, curve_form, closed_curve, self_intersect, knot_multiplicities, knots, knot_spec;
    vector<string> matches = split_parser(property);
    name = matches[0];
    degree = matches[1];
    // 列表 id 及 列表对象元素
    control_points_list = trim_brackets(matches[2]);
    // control_points_list_id = control_points_list.substr(0, (control_points_list.find(',')));
    curve_form = matches[3];
    closed_curve = matches[4];
    self_intersect = matches[5];
    knot_multiplicities = trim_brackets(matches[6]);
    knots = trim_brackets(matches[7]);
    knot_spec = matches[8];
    
    // 将 vector_node 相关属性进行压缩
    leveldb::Status status;  

    // 直接将控制点用对应坐标替代
    vector<string> matches_cartesian_point = split_parser(control_points_list);
    string control_points_list_final = "(";
    for (int i = 0; i < matches_cartesian_point.size(); i++) {
      string point_type, point_property;
      getPropertyType(client, matches_cartesian_point[i], point_type, point_property, local_compression_entitys);

      if (i == matches_cartesian_point.size() - 1) {
          control_points_list_final += split_parser(point_property)[1];
          control_points_list_final += ")";
      }
      else {
          control_points_list_final += split_parser(point_property)[1];
          control_points_list_final += ",";
      }
    }

    json entity;
    entity["entity_type"] = "B_SPLINE_CURVE_WITH_KNOTS"; 
    entity["name"] = name;
    entity["degree"] = degree;
    entity["control_points_list"] = control_points_list_final;
    // cout << control_points_list_final << endl;
    entity["curve_form"] = curve_form;
    entity["closed_curve"] = closed_curve;
    entity["self_intersect"] = self_intersect;
    entity["knot_multiplicities"] =  knot_multiplicities;
    entity["knots"] = knots;
    entity["knot_spec"] = knot_spec;

    entity["related_entity"] = "";
    entity["belong"] = "";

    return entity;
  };

  analysis["B_SPLINE_SURFACE_WITH_KNOTS"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    string name, u_degree, v_degree, control_points_list, control_points_list_id, surface_form, u_closed, v_closed, self_intersect, u_multiplicities, v_multiplicities, u_knots, v_knots, knot_spec;
    vector<string> matches = split_parser(property);
    name = matches[0];
    u_degree = matches[1];
    v_degree = matches[2];
    control_points_list = matches[3];
    // 获取 list 的索引 id
    vector<string> matches_control_points_list = split_parser(trim_brackets(control_points_list));

    string control_points_list_final = "(";
    for (int i = 0; i < matches_control_points_list.size(); i++) {
        control_points_list_final += "(";
        vector<string> points_list = split_parser(trim_brackets(matches_control_points_list[i]));
        for (int j = 0; j < points_list.size(); j++) {
          string point_type, point_property;
          getPropertyType(client, points_list[j], point_type, point_property, local_compression_entitys);
          if (j == points_list.size() - 1) {
              control_points_list_final += split_parser(point_property)[1];
              control_points_list_final += ")";
          }
          else {
              control_points_list_final += split_parser(point_property)[1];
              control_points_list_final += ",";
          }
        }
        if (i == matches_control_points_list.size() - 1) control_points_list_final += ")";
        else control_points_list_final += ",";
    }
    // 同样直接用 cartesian_point 的坐标进行替换
    surface_form = matches[4];
    u_closed = matches[5];
    v_closed = matches[6];
    self_intersect = matches[7];
    u_multiplicities = trim_brackets(matches[8]);
    v_multiplicities = trim_brackets(matches[9]);
    u_knots = trim_brackets(matches[10]);
    v_knots = trim_brackets(matches[11]);
    knot_spec = matches[12];

    json entity;
    entity["entity_type"] = "B_SPLINE_SURFACE_WITH_KNOTS";
    entity["name"] = name;
    entity["u_degree"] = u_degree;
    entity["v_degree"] = v_degree;
    entity["control_points_list"] = control_points_list_final;
    entity["surface_form"] = surface_form;
    entity["u_closed"] = u_closed;
    entity["v_closed"] = v_closed;
    entity["self_intersect"] = self_intersect;
    entity["u_multiplicities"] = u_multiplicities;
    entity["v_multiplicities"] = v_multiplicities;
    entity["u_knots"] = u_knots;
    entity["v_knots"] = v_knots;
    entity["knot_spec"] = knot_spec;

    entity["related_entity"] = "";
    entity["belong"] = "";

    return entity;
  };

  analysis["ORIENTED_EDGE"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    json entity;
    string name, oriented_edge_start_point, oriented_edge_end_point, orientation, 
                edge_curve_start_point, edge_curve_end_point, same_sense, edge_geometry_type,
                position_id, radius_1, radius_2,
                line_cartesian_point, vector_orientation, vector_magnitude;
    // 压缩的属性，并非所有实体都具有相应的值，默认为空
    position_id = "";
    radius_1 = "";
    radius_2 = "";
    line_cartesian_point = "";
    vector_orientation = "";
    vector_magnitude = "";
    string related_entity = "";


    // 处理 oriented_edge 部分的属性
    vector<string> matches_oriented_edge = split_parser(property);
    name = matches_oriented_edge[0];
    if (matches_oriented_edge[1] != "*"){
      oriented_edge_start_point = cartesian_point_compression(matches_oriented_edge[1], client, local_compression_entitys);
    }
    else oriented_edge_start_point = matches_oriented_edge[1];

    if (matches_oriented_edge[2] != "*"){
      oriented_edge_end_point = cartesian_point_compression(matches_oriented_edge[2], client, local_compression_entitys);
    }
    else oriented_edge_end_point = matches_oriented_edge[2];

    orientation = matches_oriented_edge[4];

    // 处理 edge_curve 部分的属性
    string edge_type, edge_property;
    getPropertyType(client, matches_oriented_edge[3], edge_type, edge_property, local_compression_entitys);

    vector<string> matches_edge_curve = split_parser(edge_property);
    edge_curve_start_point = cartesian_point_compression(matches_edge_curve[1], client, local_compression_entitys);
    edge_curve_end_point = cartesian_point_compression(matches_edge_curve[2], client, local_compression_entitys);
    same_sense = matches_edge_curve[4];

    // 处理 edge_geometry 部分的属性
    string edge_geometry_id = matches_edge_curve[3];
    string edge_geometry_property;
    getPropertyType(client, edge_geometry_id, edge_geometry_type, edge_geometry_property, local_compression_entitys);  

    if (edge_geometry_type == "") {
        edge_geometry_type = edge_geometry_property.substr(0, edge_geometry_property.find('('));
    }

    unordered_set<string> compression_curve_type = { "LINE", "CIRCLE", "PARABOLA", "ELLIPSE", "HYPERBOLA" };
    if (compression_curve_type.find(edge_geometry_type) != compression_curve_type.end()) {
      entity["edge_geometry_id"] = "";
      if (edge_geometry_type == "LINE") {
          vector<string> matches_line = split_parser(edge_geometry_property);
          string line_type, line_property;
          getPropertyType(client, matches_line[1], line_type, line_property, local_compression_entitys);

          line_cartesian_point = split_parser(line_property)[1];

          string vector_type, vector_property;
          getPropertyType(client, matches_line[2], vector_type, vector_property, local_compression_entitys);


          vector<string> matches_vector = split_parser(vector_property);

          vector_magnitude = matches_vector[2];

          string direction_type, direction_property;
          getPropertyType(client, matches_vector[1], direction_type, direction_property, local_compression_entitys);

          vector_orientation = split_parser(direction_property)[1];
      }
      else{
        vector<string> content_edge = split_parser(edge_geometry_property);
        position_id = content_edge[1];
        if ( content_edge.size() > 2 ){
          radius_1 = content_edge[2];
          if ( content_edge.size() > 3 )
            radius_2 = content_edge[3];
        }
      }

    }
    else {
      entity["edge_geometry_id"] = '[' + edge_geometry_id + ']';
      related_entity = related_entity + edge_geometry_id;
      // 找到第一个匹配的对象
      auto it = std::find(local_compression_entitys.begin(), local_compression_entitys.end(), leftPad(edge_geometry_id, padding));
      if (it != local_compression_entitys.end()) {
          local_compression_entitys.erase(it); // 删除第一个匹配的对象
      }
    }

    entity["entity_type"] = "ORIENTED_EDGE"; 
    entity["name"] = name;
    entity["oriented_edge_start_point"] = oriented_edge_start_point;
    entity["oriented_edge_end_point"] = oriented_edge_end_point;
    entity["orientation"] = orientation;
    entity["edge_curve_start_point"] = edge_curve_start_point;
    entity["edge_curve_end_point"] = edge_curve_end_point;
    entity["same_sense"] = same_sense;
    entity["edge_geometry_type"] = edge_geometry_type;
    entity["position_id"] = '[' + position_id + ']';
    entity["radius_1"] = radius_1;
    entity["radius_2"] = radius_2;
    entity["line_cartesian_point"] = line_cartesian_point;
    entity["vector_orientation"] = vector_orientation;
    entity["vector_magnitude"] = vector_magnitude;

    if (position_id != ""){
      if (related_entity != "")
        related_entity = related_entity + ',' + position_id;
      else
        related_entity = position_id;
    }    
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity;
  };

  analysis["LINE"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    string name, line_cartesian_point, vector_magnitude, vector_orientation;
    vector<string> matches = split_parser(property);
    name = matches[0];

    string point_type, point_property;
    getPropertyType(client, matches[1], point_type, point_property, local_compression_entitys);
    line_cartesian_point = split_parser(point_property)[1];

    string vector_type, vector_property;
    getPropertyType(client, matches[2], vector_type, vector_property, local_compression_entitys);
    vector<string> matches_vector = split_parser(vector_property);
    vector_magnitude = matches_vector[2];

    string direction_type, direction_property;
    getPropertyType(client, matches_vector[1], direction_type, direction_property, local_compression_entitys);
    vector_orientation = split_parser(direction_property)[1];

    json entity;
    entity["name"] = name;
    entity["entity_type"] = "LINE";
    entity["line_cartesian_point"] = line_cartesian_point;
    entity["line_cartesian_point"] = line_cartesian_point;
    entity["vector_magnitude"] = vector_magnitude;
    entity["vector_orientation"] = vector_orientation;

    entity["related_entity"] = "";
    entity["belong"] = "";

    return entity;
  };

  analysis["CIRCLE"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "CIRCLE";
    entity["name"] = matches[0];
    entity["position_id"] = '[' + matches[1] + ']';
    entity["radius"] = matches[2];

    entity["related_entity"] = matches[1];    
    entity["belong"] = "";

    return entity;
  };

  analysis["PARABOLA"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "PARABOLA";
    entity["name"] = matches[0];
    entity["position_id"] = '[' + matches[1] + ']';
    entity["distance"] = matches[2];

    entity["related_entity"] = matches[1];
    entity["belong"] = "";

    return entity;
  };

  analysis["ELLIPSE"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "ELLIPSE";
    entity["name"] = matches[0];
    entity["position_id"] = '[' + matches[1] + ']';
    entity["radius_1"] =  matches[2];
    entity["radius_2"] = matches[3];

    entity["related_entity"] = matches[1];
    entity["belong"] = "";

    return entity;
  };

  analysis["HYPERBOLA"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "HYPERBOLA";
    entity["name"] = matches[0];
    entity["position_id"] = '[' + matches[1] + ']';
    entity["semi_axis"] = matches[2];
    entity["semi_imag_axis"] = matches[3];

    entity["related_entity"] = matches[1];
    entity["belong"] = "";

    return entity;
  };

  analysis["SURFACE_CURVE"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);
    string related_entity = "";

    json entity;
    entity["entity_type"] = "SURFACE_CURVE";
    entity["name"] = matches[0];
    entity["curve_3d_id"] = '[' + matches[1] + ']';
    entity["associated_geometry_ids_list"] = '[' + trim_brackets(matches[2]) + ']';
    entity["master_representation"] = matches[3];
    
    related_entity = related_entity + matches[1] + ',';
    related_entity = related_entity + trim_brackets(matches[2]);
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity;
  };

  analysis["PCURVE"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
      vector<string> matches = split_parser(property);
      string related_entity = "";

      json entity;
      entity["entity_type"] = "PCURVE";
      entity["name"] = matches[0];
      entity["reference_to_curve_id"] = '[' + matches[2] + ']';

      // 涉及压缩的对象 matches[1]
      string basis_surface_id, face_geometry_type, position_id, radius_1, radius_2;
      basis_surface_id = matches[1];
      position_id = "";
      radius_1 = "";
      radius_2 = "";

      // 需要进行压缩时根据记录的 geometry_face_id 来查找数据库
      string geometry_face_property;
      // entity_type(property) --> entity_type, property
      getPropertyType(client, basis_surface_id, face_geometry_type, geometry_face_property, local_compression_entitys);

      unordered_set<string> compression_surface_type = { "PLANE", "", "SPHERICAL_SURFACE", "CONICAL_SURFACE", "TOROIDAL_SURFACE" };

      if (face_geometry_type == "") {
          face_geometry_type = geometry_face_property.substr(0, geometry_face_property.find('('));
      }
      
      if (compression_surface_type.find(face_geometry_type) != compression_surface_type.end()) {
        entity["basis_surface_id"] = "";
        // property -> 具体属性值
        vector<string> matches_geometry_face_property = split_parser(geometry_face_property);
        position_id = matches_geometry_face_property[1];

        if (matches_geometry_face_property.size() > 2){
          radius_1 = matches_geometry_face_property[2];
          if (matches_geometry_face_property.size() > 3)
            radius_2 = matches_geometry_face_property[3];
        }
      }
      else {
        entity["basis_surface_id"] = '[' + basis_surface_id + ']';
        related_entity = related_entity + basis_surface_id + ','; 
        // 找到第一个匹配的对象
        auto it = std::find(local_compression_entitys.begin(), local_compression_entitys.end(), leftPad(basis_surface_id, padding));
        if (it != local_compression_entitys.end()) {
            local_compression_entitys.erase(it); // 删除第一个匹配的对象
        }
      }
      
      entity["position_id"] = '[' + position_id + ']';
      entity["face_geometry_type"] = face_geometry_type;
      entity["radius_1"] = radius_1;
      entity["radius_2"] = radius_2;
      if (position_id != "")
        related_entity = related_entity + position_id + ',';
      related_entity = related_entity + matches[2];
      entity["related_entity"] = related_entity;

      entity["belong"] = "";

      return entity;
  };

  analysis["TRIMMED_CURVE"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "TRIMMED_CURVE";
    entity["name"] = matches[0];
    entity["basis_curve"] =  matches[1];
    entity["trim_1"] = matches[2];
    entity["trim_2"] = matches[3];
    entity["sense_agreement"] = matches[4];
    entity["master_representation"] = matches[5];

    entity["related_entity"] = matches[1];
    entity["belong"] = "";

    return entity; 
  };  


  analysis["DEFINITIONAL_REPRESENTATION"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);
    string related_entity = "";

    json entity;
    entity["entity_type"] = "DEFINITIONAL_REPRESENTATION";
    entity["name"] = matches[0];
    entity["item_ids_list"] = '[' + trim_brackets(matches[1]) + ']';
    entity["context_of_items_id"] = '[' + matches[2] + ']';

    related_entity = related_entity + trim_brackets(matches[1]) + ',';
    related_entity = related_entity + matches[2];
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity;   
  };

  analysis["FACE_BOUND"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "FACE_BOUND";
    entity["name"] = matches[0];
    entity["loop_id"] = '[' + matches[1] + ']';
    entity["orientation"] = matches[2];

    entity["related_entity"] = matches[1];
    entity["belong"] = "";

    return entity;  
  }; 

  analysis["FACE_OUTER_BOUND"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "FACE_OUTER_BOUND";
    entity["name"] = matches[0];
    entity["loop_id"] = '[' + matches[1] + ']';
    entity["orientation"] = matches[2];

    entity["related_entity"] = matches[1];
    entity["belong"] = "";

    return entity;  
  };

  analysis["EDGE_LOOP"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "EDGE_LOOP";
    entity["name"] = matches[0];
    entity["ids"] = '[' + trim_brackets(matches[1]) + ']';

    entity["related_entity"] = trim_brackets(matches[1]);
    entity["belong"] = "";

    return entity; 
  };

  analysis["VERTEX_LOOP"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "VERTEX_LOOP";
    entity["name"] = matches[0];
    entity["loop_vertex"] = cartesian_point_compression(matches[1], client, local_compression_entitys);

    entity["related_entity"] = "";
    entity["belong"] = "";

    return entity; 
  };

  analysis["POLY_LOOP"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    string name, polygon_points_list;
    vector<string> matches = split_parser(property);
    name = matches[0];
    // 直接替换为对应的坐标
    polygon_points_list = trim_brackets(matches[1]);

    string polygon_cartesian_points_list = "(";

    vector<string> cartesian_points = split_parser(polygon_points_list);
    for (int i = 0; i < cartesian_points.size(); i++) {
        string point_type, point_property;
        getPropertyType(client, cartesian_points[i], point_type, point_property, local_compression_entitys);
        polygon_cartesian_points_list += split_parser(point_property)[1];
        if (i == cartesian_points.size() - 1) polygon_cartesian_points_list += ")";
        else polygon_cartesian_points_list += ",";
    }

    json entity;
    entity["entity_type"] = "POLY_LOOP";
    entity["name"] = name;
    entity["polygon_points_list"] = polygon_cartesian_points_list;

    entity["related_entity"] = "";
    entity["belong"] = "";

    return entity; 
  };

  analysis["AXIS2_PLACEMENT_3D"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    string name, location_cartesian_point_name, location_cartesian_point, axis_name, axis, ref_direction_name = "", ref_direction = "";
    vector<string> matches = split_parser(property);
    name = matches[0];

    // 直接用 location_cartesian_point 坐标替换
    string point_type, point_property;
    getPropertyType(client, matches[1], point_type, point_property, local_compression_entitys);
    vector<string> matches_location_point = split_parser(point_property);
    location_cartesian_point_name = matches_location_point[0];
    location_cartesian_point = matches_location_point[1];

    // 直接用 坐标轴 来进行替换
    string axis_type, axis_property;
    getPropertyType(client, matches[2], axis_type, axis_property, local_compression_entitys);
    vector<string> matches_axis = split_parser(axis_property);
    axis_name = matches_axis[0];
    axis = matches_axis[1];

    string ref_type, ref_property;
    if (matches[3] != "$"){
      getPropertyType(client, matches[3], ref_type, ref_property, local_compression_entitys);
      vector<string> matches_ref_direction = split_parser(ref_property);
      ref_direction_name = matches_ref_direction[0];
      ref_direction = matches_ref_direction[1];
    }

    json entity;
    entity["entity_type"] = "AXIS2_PLACEMENT_3D";
    entity["name"] = name;
    entity["location_cartesian_point_name"] = location_cartesian_point_name;
    entity["location_cartesian_point"] = location_cartesian_point;
    entity["axis_name"] = axis_name;
    entity["axis"] = axis;
    entity["ref_direction_name"] = ref_direction_name;
    entity["ref_direction"] = ref_direction;
    
    entity["related_entity"] = "";
    entity["belong"] = "";

    return entity;    
  };

  analysis["AXIS2_PLACEMENT_2D"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    string name, location_cartesian_point_name, location_cartesian_point, axis_name, axis;
    vector<string> matches = split_parser(property);
    name = matches[0];

    // 直接用 location_cartesian_point 坐标替换
    string point_type, point_property;
    getPropertyType(client, matches[1], point_type, point_property, local_compression_entitys);
    vector<string> matches_location_point = split_parser(point_property);
    location_cartesian_point_name = matches_location_point[0];
    location_cartesian_point = matches_location_point[1];

    // 直接用 坐标轴 来进行替换
    string axis_type, axis_property;
    getPropertyType(client, matches[2], axis_type, axis_property, local_compression_entitys);
    vector<string> matches_axis = split_parser(axis_property);
    axis_name = matches_axis[0];
    axis = matches_axis[1];

    json entity;
    entity["entity_type"] = "AXIS2_PLACEMENT_2D";
    entity["name"] = name;
    entity["location_cartesian_point_name"] = location_cartesian_point_name;
    entity["location_cartesian_point"] = location_cartesian_point;
    entity["axis_name"] = axis_name;
    entity["axis"] = axis;

    entity["related_entity"] = "";
    entity["belong"] = "";

    return entity;      
  };


  analysis["AXIS1_PLACEMENT"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    string name, location_cartesian_point_name, location_cartesian_point, axis_name, axis;
    vector<string> matches = split_parser(property);
    name = matches[0];

    // 直接用 location_cartesian_point 坐标替换
    string point_type, point_property;
    getPropertyType(client, matches[1], point_type, point_property, local_compression_entitys);
    vector<string> matches_location_point = split_parser(point_property);
    location_cartesian_point_name = matches_location_point[0];
    location_cartesian_point = matches_location_point[1];

    // 直接用 坐标轴 来进行替换
    string axis_type, axis_property;
    getPropertyType(client, matches[2], axis_type, axis_property, local_compression_entitys);
    vector<string> matches_axis = split_parser(axis_property);
    axis_name = matches_axis[0];
    axis = matches_axis[1];

    json entity;
    entity["entity_type"] = "AXIS1_PLACEMENT";
    entity["name"] = name;
    entity["location_cartesian_point_name"] = location_cartesian_point_name;
    entity["location_cartesian_point"] = location_cartesian_point;
    entity["axis_name"] = axis_name;
    entity["axis"] = axis;

    entity["related_entity"] = "";
    entity["belong"] = "";

    return entity;      
  };

  analysis["STYLED_ITEM"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);
    string related_entity = "";

    json entity;
    entity["entity_type"] = "STYLED_ITEM";
    entity["name"] = matches[0];
    entity["style_ids"] = '[' + trim_brackets(matches[1]) + ']';
    entity["item_id"] = '[' + matches[2] + ']';

    related_entity = related_entity + trim_brackets(matches[1]) + ',';
    related_entity = related_entity + matches[2];
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity; 
  };

  analysis["MECHANICAL_DESIGN_GEOMETRIC_PRESENTATION_REPRESENTATION"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);
    string related_entity = "";

    json entity;
    entity["entity_type"] = "MECHANICAL_DESIGN_GEOMETRIC_PRESENTATION_REPRESENTATION";
    entity["name"] = matches[0];
    entity["styled_item_ids"] = '[' + trim_brackets(matches[1]) + ']';
    entity["context_of_items"] = '[' + matches[2] + ']';

    related_entity = related_entity + trim_brackets(matches[1]) + ',';
    related_entity = related_entity + matches[2];
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity; 
  };

  analysis["PRESENTATION_STYLE_ASSIGNMENT"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "PRESENTATION_STYLE_ASSIGNMENT";
    entity["style_ids"] = '[' + trim_brackets(matches[0]) + ']';

    entity["related_entity"] = trim_brackets(matches[0]);  
    entity["belong"] = "";

    return entity; 
  };

  analysis["SURFACE_STYLE_USAGE"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "SURFACE_STYLE_USAGE";
    entity["side_chosen"] = matches[0];
    entity["surface_side_style_id"] = '[' + matches[1] + ']';

    entity["related_entity"] = matches[1];  
    entity["belong"] = "";

    return entity; 
  };

  analysis["SURFACE_SIDE_STYLE"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "SURFACE_SIDE_STYLE";
    entity["name"] = matches[0];
    entity["surface_style_element_select_ids_list"] = '[' + trim_brackets(matches[1]) + ']';

    entity["related_entity"] = trim_brackets(matches[1]);  
    entity["belong"] = "";

    return entity; 
  };

  analysis["SURFACE_STYLE_FILL_AREA"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "SURFACE_STYLE_FILL_AREA";
    entity["fill_area_id"] = '[' + matches[0] + ']';

    entity["related_entity"] = matches[0];  
    entity["belong"] = "";

    return entity; 
  };

  analysis["FILL_AREA_STYLE"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "FILL_AREA_STYLE";
    entity["name"] = matches[0];
    entity["fill_style_colour_ids_list"] = '[' + trim_brackets(matches[1]) + ']';

    entity["related_entity"] = trim_brackets(matches[1]);  
    entity["belong"] = "";

    return entity; 
  };

  analysis["FILL_AREA_STYLE_COLOUR"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "FILL_AREA_STYLE_COLOUR";
    entity["name"] = matches[0];
    entity["fill_color_id"] = '[' + matches[1] + ']';

    entity["related_entity"] = matches[1];  
    entity["belong"] = "";

    return entity; 
  };

  analysis["DRAUGHTING_PRE_DEFINED_COLOUR"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "DRAUGHTING_PRE_DEFINED_COLOUR";
    entity["name"] = matches[0];

    entity["related_entity"] = "";
    entity["belong"] = "";

    return entity; 
  };

  analysis["COLOUR_RGB"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    string name, red, green, blue;
    vector<string> matches = split_parser(property);
    name = matches[0];
    red = matches[1];
    green = matches[2];
    blue = matches[3];

    json entity;
    entity["entity_type"] = "COLOUR_RGB";
    entity["name"] =  matches[0];
    entity["red"] = matches[1];
    entity["green"] = matches[2];
    entity["blue"] = matches[3];

    entity["related_entity"] = "";
    entity["belong"] = "";

    return entity; 
  };

  analysis["SURFACE_STYLE_RENDERING_WITH_PROPERTIES"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);
    string related_entity = "";

    json entity;
    entity["entity_type"] = "SURFACE_STYLE_RENDERING_WITH_PROPERTIES";
    entity["rendering_method"] = matches[0];
    entity["surface_colour_id"] = '[' + matches[1] + ']';
    entity["properties"] = '[' + trim_brackets(matches[2]) + ']';

    related_entity = related_entity + matches[1] + ',';
    related_entity = related_entity + trim_brackets(matches[2]);
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity; 
  };

  analysis["SURFACE_STYLE_TRANSPARENT"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "SURFACE_STYLE_TRANSPARENT";
    entity["transparency"] = matches[0];

    entity["related_entity"] = "";
    entity["belong"] = "";

    return entity; 
  };

  analysis["CONTEXT_DEPENDENT_SHAPE_REPRESENTATION"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);
    string related_entity = "";

    json entity;
    entity["entity_type"] = "CONTEXT_DEPENDENT_SHAPE_REPRESENTATION";
    entity["representation_relation_id"] = '[' + matches[0] + ']';
    entity["represented_product_relation_id"] = '[' + matches[1] + ']';

    related_entity = related_entity + matches[0] + ',';
    related_entity = related_entity + matches[1];
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity;    
  };

  analysis["PRODUCT_RELATED_PRODUCT_CATEGORY"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "PRODUCT_RELATED_PRODUCT_CATEGORY";
    entity["name"] = matches[0];
    entity["description"] = matches[1];
    entity["product_ids_list"] = '[' + trim_brackets(matches[2]) + ']';

    entity["related_entity"] = trim_brackets(matches[2]);
    entity["belong"] = "";

    return entity;    
  };

  analysis["PRODUCT_DEFINITION_SHAPE"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    string name, description, product_definition_id;
    vector<string> matches = split_parser(property);
    name = matches[0];
    description = matches[1];
    product_definition_id = matches[2];

    json entity;
    entity["entity_type"] = "PRODUCT_DEFINITION_SHAPE";
    entity["name"] = matches[0];
    entity["description"] = matches[1];
    entity["product_definition_id"] = '[' + matches[2] + ']';

    entity["related_entity"] = matches[2];
    entity["belong"] = "";

    return entity;   
  };

  analysis["PRODUCT_DEFINITION"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    string name, description, formation_id, frame_id_of_reference;
    vector<string> matches = split_parser(property);
    name = matches[0];
    description = matches[1];
    formation_id = matches[2];
    frame_id_of_reference = matches[3];
    string related_entity = "";

    json entity;
    entity["entity_type"] = "PRODUCT_DEFINITION";
    entity["name"] = matches[0];
    entity["description"] = matches[1];
    entity["formation_id"] = '[' + matches[2] + ']';
    entity["frame_id_of_reference"] = '[' + matches[3] + ']';

    related_entity = related_entity + matches[2] + ',';
    related_entity = related_entity + matches[3];
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity;    
  };

  analysis["PRODUCT_DEFINITION_FORMATION"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    string name, description, of_product_id;
    vector<string> matches = split_parser(property);
    name = matches[0];
    description = matches[1];
    of_product_id = matches[2];

    json entity;
    entity["entity_type"] = "PRODUCT_DEFINITION_FORMATION";
    entity["name"] = matches[0];
    entity["description"] = matches[1];
    entity["of_product_id"] = '[' + matches[2] + ']';

    entity["related_entity"] = matches[2];
    entity["belong"] = "";

    return entity;    
  };

  analysis["PRODUCT_DEFINITION_CONTEXT"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);
    string related_entity = "";

    json entity;
    entity["entity_type"] = "PRODUCT_DEFINITION_CONTEXT";
    entity["name"] = matches[0];
    entity["life_cycle_stage"] = matches[2];
    entity["frame_id_of_reference"] = '[' + matches[1] + ']';

    entity["related_entity"] = matches[1];
    entity["belong"] = "";

    return entity; 
  };

  analysis["PRODUCT"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);
    string related_entity = "";

    json entity;
    entity["entity_type"] = "PRODUCT";
    entity["name"] = matches[1];
    entity["product_id"] = matches[0];
    // cout << matches[1] << "   " << matches[0] << endl;
    entity["description"] = matches[2];
    entity["frame_ids_of_reference"] = '[' + trim_brackets(matches[3]) + ']';
    related_entity = related_entity + trim_brackets(matches[3]);
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity; 
  };

  analysis["PRODUCT_CONTEXT"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "PRODUCT_CONTEXT";
    entity["name"] = matches[0];
    entity["frame_of_reference_id"] = '[' + matches[1] + ']';
    entity["discipline_type"] = matches[2];

    entity["related_entity"] = matches[1];
    entity["belong"] = "";
     
    return entity; 
  };  

  analysis["APPLICATION_CONTEXT"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "APPLICATION_CONTEXT";
    entity["application"] = matches[0];

    entity["related_entity"] = "";
    entity["belong"] = "";

    return entity;     
  };

  analysis["APPLICATION_PROTOCOL_DEFINITION"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);
    
    json entity;
    entity["entity_type"] = "APPLICATION_PROTOCOL_DEFINITION";
    entity["status"] = matches[0];
    entity["application_interpreted_model_schema_name"] = matches[1];
    entity["application_protocol_year"] = matches[2];
    entity["application_context_id"] = '[' + matches[3] + ']';

    entity["related_entity"] = matches[3];    
    entity["belong"] = "";

    return entity; 
  };

  analysis["NEXT_ASSEMBLY_USAGE_OCCURRENCE"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);
    string related_entity = "";

    json entity;
    entity["entity_type"] = "NEXT_ASSEMBLY_USAGE_OCCURRENCE";
    entity["name"] = matches[1];
    entity["assembly_id"] = matches[0];
    entity["description"] = matches[2];
    entity["relating_product_definition_id"] = '[' + matches[3] + ']';
    entity["related_product_definition_id"] = '[' + matches[4] + ']';
    entity["reference_designator"] = matches[5];

    related_entity = related_entity + matches[3] + ',';
    related_entity = related_entity + matches[4];
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity; 
  };

  analysis["PROPERTY_DEFINITION_REPRESENTATION"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);
    string related_entity = "";

    json entity;
    entity["entity_type"] = "PROPERTY_DEFINITION_REPRESENTATION";
    entity["property_definition_id"] = '[' + matches[0] + ']';
    entity["used_representation_id"] = '[' + matches[1] + ']';

    related_entity = related_entity + matches[0] + ',';
    related_entity = related_entity + matches[1];
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity; 
  };

  analysis["PROPERTY_DEFINITION"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "PROPERTY_DEFINITION";
    entity["name"] = matches[0];
    entity["description"] = matches[1];
    entity["definition_id"] = '[' + matches[2] + ']';

    entity["related_entity"] = matches[2];
    entity["belong"] = "";

    return entity;
  };

  analysis["DEFINITIONAL_REPRESENTATION"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);
    string related_entity = "";

    json entity;
    entity["entity_type"] = "DEFINITIONAL_REPRESENTATION";
    entity["name"] = matches[0];
    entity["context_id_of_items"] = '[' + matches[2] + ']';
    entity["item_ids_list"] = '[' + trim_brackets(matches[1]) + ']';

    related_entity = related_entity + matches[2] + ',';
    related_entity = related_entity + trim_brackets(matches[1]) ;
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity;
  };

  analysis["REPRESENTATION"] = [](string property, LsmdbClient &client,unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);
    string related_entity = "";

    json entity;
    entity["entity_type"] = "REPRESENTATION";
    entity["name"] = matches[0];
    entity["descriptive_item_ids"] = '[' + trim_brackets(matches[1]) + ']';
    entity["context_of_item_id"] = '[' + matches[2] + ']';

    related_entity = related_entity + trim_brackets(matches[1]) + ',';
    related_entity = related_entity + matches[2]  ;
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity;
  }; 

  analysis["DESCRIPTIVE_REPRESENTATION_ITEM"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "DESCRIPTIVE_REPRESENTATION_ITEM";
    entity["name"] = matches[0];
    entity["description"] = matches[1];

    entity["related_entity"] = "";
    entity["belong"] = "";

    return entity; 
  };

  analysis["GENERAL_PROPERTY"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "GENERAL_PROPERTY";
    entity["name"] = matches[1];
    entity["description"] =  matches[2];
    entity["general_property_id"] = matches[0];

    entity["related_entity"] = "";
    entity["belong"] = "";

    return entity; 
  };  



  // 处理类型名为空的对象
  analysis[""] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    string content_node;
    // 提前将需要压缩的点对象
    string related_entity = "";

    size_t pos = property.find('(');
    if (pos != string::npos){
      string prefix = property.substr(0, pos);
      prefix = trim(prefix); // 删除多余的空格
      if (prefix == "BOUNDED_CURVE" || prefix == "BOUNDED_SURFACE"){
        std::regex pattern("#(\\d+)");
        std::smatch match;
        string::const_iterator searchStart(property.cbegin());

        while (std::regex_search(searchStart, property.cend(), match, pattern)) {
            // 将匹配到的数字部分转换为整数并存储
            related_entity = related_entity + match[1].str() + ',';
            local_compression_entitys.insert(leftPad(match[1].str(), 16));
            searchStart = match.suffix().first; // 更新搜索起点
        }
      }
      if (prefix == "REPRESENTATION_RELATIONSHIP"){
        std::regex pattern("#(\\d+)");
        std::smatch match;
        string::const_iterator searchStart(property.cbegin());
        
        while (std::regex_search(searchStart, property.cend(), match, pattern)) {
          // 将匹配到的数字部分转换为整数并存储
          related_entity = match[1].str();
          json entity;
          entity["entity_type"] = "REPRESENTATION_RELATIONSHIP";
          // entity["related_entity"] = related_entity.substr(0, related_entity.size() - 1);
          entity["related_entity"] = related_entity;
          entity["belong"] = "";
           
          return entity; 
        }
    }
  }

    /*
    vector<string> matches = split_parser(property);
    content_node = matches[0];  // 暂时先直接存储，后续可能以 json 形式存入其他数据库中
    */
    json entity;
    entity["entity_type"] = "";
    entity["content_node"] = property;

    // entity["related_entity"] = related_entity.substr(0, related_entity.size() - 1);
    entity["related_entity"] = "";
    entity["belong"] = "";

    return entity; 
  };

  analysis["UNCERTAINTY_MEASURE_WITH_UNIT"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "UNCERTAINTY_MEASURE_WITH_UNIT";
    entity["name"] = matches[2];
    entity["value_component"] = matches[0];
    entity["unit_component_id"] = '[' + matches[1] + ']';
    entity["description"] = matches[3];

    entity["related_entity"] = matches[1];
    entity["belong"] = "";

    return entity; 
  };

  analysis["CARTESIAN_POINT"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);

    json entity;
    entity["entity_type"] = "CARTESIAN_POINT";
    entity["name"] = matches[0];
    entity["coordinate"] = matches[1];

    entity["related_entity"] = "";
    entity["belong"] = "";

    return entity;    
  };

  analysis["ADVANCED_BREP_SHAPE_REPRESENTATION"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);
    string related_entity = "";

    json entity;
    entity["entity_type"] = "ADVANCED_BREP_SHAPE_REPRESENTATION";
    entity["name"] = matches[0];
    entity["items"] = '[' + trim_brackets(matches[1]) + ']';
    entity["context_of_items_id"] = '[' + matches[2] + ']';

    related_entity = related_entity + trim_brackets(matches[1])  + ',';
    related_entity = related_entity + matches[2];
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity;   
  };

  analysis["SHAPE_DEFINITION_REPRESENTATION"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    string product_definition_shape_id, used_representation_id;
    vector<string> matches = split_parser(property);
    product_definition_shape_id = matches[0];
    used_representation_id = matches[1];
    string related_entity = "";

    json entity;
    entity["entity_type"] = "SHAPE_DEFINITION_REPRESENTATION";
    entity["product_definition_shape_id"] = '[' + matches[0] + ']';
    entity["used_representation_id"] = '[' + matches[1] + ']';

    related_entity = related_entity + matches[0]  + ',';
    related_entity = related_entity + matches[1];
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity; 
  };

  analysis["SHAPE_REPRESENTATION_RELATIONSHIP"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);
    string related_entity = "";

    json entity;
    entity["entity_type"] = "SHAPE_REPRESENTATION_RELATIONSHIP";
    entity["name"] = matches[0];
    entity["description"] =  matches[1];
    entity["representaion_id"] = '[' + matches[2] + ']';
    entity["representation_reference_id"] = '[' + matches[3] + ']';

    related_entity = related_entity + matches[2]  + ',';
    related_entity = related_entity + matches[3] ;
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity; 
  };

  analysis["ADVANCED_BREP_SHAPE_REPRESENTATION"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    vector<string> matches = split_parser(property);
    string related_entity = "";

    json entity;
    entity["entity_type"] = "ADVANCED_BREP_SHAPE_REPRESENTATION";
    entity["name"] = matches[0];
    entity["manifold_solid_brep_ids_list"] = '[' + trim_brackets(matches[1]) + ']';
    entity["context_of_items_id"] = '[' + matches[2] + ']';

    related_entity = related_entity + trim_brackets(matches[1])   + ',';
    related_entity = related_entity + matches[2] ;
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity; 
  };

  analysis["MANIFOLD_SURFACE_SHAPE_REPRESENTATION"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    string name, shell_based_surface_model_ids_list, context_of_items_id;
    vector<string> matches = split_parser(property);
    name = matches[0];
    shell_based_surface_model_ids_list = trim_brackets(matches[1]);
    context_of_items_id = matches[2];
    string related_entity = "";
    

    json entity;
    entity["entity_type"] = "MANIFOLD_SURFACE_SHAPE_REPRESENTATION";
    entity["name"] = matches[0];
    entity["shell_based_surface_model_ids_list"] = '[' + trim_brackets(matches[1]) + ']';
    entity["context_of_items_id"] = '[' + matches[2] + ']';

    related_entity = related_entity + trim_brackets(matches[1])   + ',';
    related_entity = related_entity + matches[2];
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity; 
  };

  analysis["SHAPE_REPRESENTATION"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    string name, axis_placement_item_id_list, context_of_items_id;
    vector<string> matches = split_parser(property);
    name = matches[0];
    axis_placement_item_id_list = trim_brackets(matches[1]);
    context_of_items_id = matches[2];
    string related_entity = "";

    json entity;
    entity["entity_type"] = "SHAPE_REPRESENTATION";
    entity["name"] = matches[0];
    entity["axis_placement_item_id_list"] = '[' + trim_brackets(matches[1]) + ']';
    entity["context_of_items_id"] = '[' + matches[2] + ']';

    related_entity = related_entity + trim_brackets(matches[1])   + ',';
    related_entity = related_entity + matches[2];
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity; 
  }; 

  analysis["ITEM_DEFINED_TRANSFORMATION"] = [](string property, LsmdbClient &client, unordered_set<string> &local_compression_entitys) {
    string name, description, transform_item_1_id, transform_item_2_id;
    vector<string> matches = split_parser(property);
    name = matches[0];
    description = matches[1];
    transform_item_1_id = matches[2];
    transform_item_2_id = matches[3];
    string related_entity = "";

    json entity;
    entity["entity_type"] = "ITEM_DEFINED_TRANSFORMATION";
    entity["name"] = matches[0];
    entity["description"] = matches[1];
    entity["transform_item_1_id"] = '[' + matches[2] + ']';
    entity["transform_item_2_id"] = '[' + matches[3] + ']';

    related_entity = related_entity + matches[2]   + ',';
    related_entity = related_entity + matches[3];
    entity["related_entity"] = related_entity;
    entity["belong"] = "";

    return entity;      
  };
}


// 对 property 进行解析，并且根据 entity_name 生成不同的 json 存储
void entityPropertyJson(LsmdbClient &client, string entity_id, string entity_name, string property, unordered_set<string> &local_compression_entitys, unordered_set<string> &local_part_entity_id, unordered_set<string> &local_context_entity_id, string stepFilePath){
  // 使用 unordered_map 实现 switch-case 来实现基于 entity_name 来选择相应的处理函数
  json entity;
  string json_content;
  if (analysis.find(entity_name) != analysis.end()){
    // cout << entity_id << "   " << entity_name << endl;
    entity= analysis[entity_name](property, client, local_compression_entitys);

    try {
      json_content = entity.dump();
    } catch (nlohmann::json::type_error& e){
      cout << property << endl;
      entity["name"] = "";
      if (entity_name == "PRODUCT"){
        entity["product_id"] = "";
      }
      json_content = entity.dump();
      // std::cerr << "Caught JSON type_error: " << e.what() << std::endl;

    }

    // 添加文件名信息至 entity_id
    entity_id = stepFilePath + entity_id;

    // 对划分实体标志单元的 entity_id 进行记录
    if ( entity_name == "ADVANCED_BREP_SHAPE_REPRESENTATION" || entity_name == "MANIFOLD_SURFACE_SHAPE_REPRESENTATION"){
      local_part_entity_id.insert(entity_id);
    }

    if ( entity_name == "CONTEXT_DEPENDENT_SHAPE_REPRESENTATION" ){
      local_context_entity_id.insert(entity_id);
    }


    // 将 json 写入 levelDB
    bool status = client.Put("json_" + entity_id, json_content);
    if (!status) 
      std::cerr << "数据存储失败: " << "entity_id: " << entity_id << std::endl;
  }

}


// 获取到逐个实体的 entity_id = entity_name(property)
void entityAnalysis(LsmdbClient &client, string entity_id, string entity_content, unordered_set<string> &local_compression_entitys, unordered_set<string> &local_part_entity_id, unordered_set<string> &local_context_entity_id, string stepFilePath){
  string entity_name;
  string property;

  parse_content(entity_content, entity_name, property);

  entityPropertyJson(client, entity_id, entity_name, property, local_compression_entitys, local_part_entity_id, local_context_entity_id, stepFilePath);
}

std::string commonPrefix(const std::string& str1, const std::string& str2) {
    size_t minLen = std::min(str1.length(), str2.length());
    size_t i = 0;
    while (i < minLen && str1[i] == str2[i]) {
        ++i;
    }
    return str1.substr(0, i);
}

// 实现范围查询
void rangeQuery(LsmdbClient &client, const std::string start_key, const std::string end_key, unordered_set<string>& delete_id, unordered_set<string> &part_entity_id, unordered_set<string> &context_entity_id, string stepFilePath) {
    std::string prefix = commonPrefix(start_key, end_key);
    std::vector<std::pair<std::string, std::string>> prefixData = client.GetDataWithPrefix(prefix);

    // 局部需要删除的点
    unordered_set<string> local_compression_entitys;
    unordered_set<string> local_part_entity_id;
    unordered_set<string> local_context_entity_id;


    // 使用迭代器从 start_key 进行查找
    // for (it->Seek(start_key); it->Valid() && it->key().ToString() <= end_key; it->Next()) {
    for (const auto& kv : prefixData) {
        // 对实体进行解析
        // auto begin_analysis = std::chrono::high_resolution_clock::now();
        entityAnalysis(client, kv.first, kv.second, local_compression_entitys, local_part_entity_id, local_context_entity_id, stepFilePath);
        // auto end_analysis = std::chrono::high_resolution_clock::now();
  
        // 计算执行时长
        // std::chrono::duration<double> duration_rangeQuery = begin_analysis - end_analysis;
        // std::cout << "解析实体 : " << it->key().ToString() << "  的开销为" << duration_rangeQuery.count() << " 秒" << std::endl;
    }
    // 合并局部结果到全局结果中
    // 压缩后需要删除的局部结果合并
    {
      lock_guard<mutex> lock(dbjson_delete_mutex);
      delete_id.insert(local_compression_entitys.begin(), local_compression_entitys.end());
    }

    // 检索到的划分标志entity_id合并
    {
      lock_guard<mutex> lock(part_entity_mutex);
      part_entity_id.insert(local_part_entity_id.begin(), local_part_entity_id.end());
    }

    // 检索到的划分标志context_id合并
    {
      lock_guard<mutex> lock(context_entity_mutex);
      context_entity_id.insert(local_context_entity_id.begin(), local_context_entity_id.end());
    }
}

string serializeUnorderedSet(const unordered_set<std::string>& set) {
    std::ostringstream oss;
    for (const auto& elem : set) {
        oss << elem << ";"; // 使用分号作为分隔符
    }
    return oss.str();
}

void deleteEntity(const std::unordered_set<std::string>::iterator start,
                  const std::unordered_set<std::string>::iterator end,
                  string fileName, leveldb::DB* dbJson){
  // item 对象为待删除的entity_id
  for (auto item = start; item != end; item ++){
    string entity_id = fileName + *item; // 非 levelDB 中的 key 形式，需要先进行转换
    
    leveldb::Status delete_status = dbJson->Delete(leveldb::WriteOptions(), entity_id);
    if (!delete_status.ok()) {
      std::cerr << "Unable to delete key: " << delete_status.ToString() << std::endl;
    }
  }
}

void splitPart(const std::unordered_set<std::string>::iterator start, const std::unordered_set<std::string>::iterator end, string fileName, LsmdbClient &client){
  // item 对象为拆分的标志性实体的 id
  for (auto item = start; item != end; item ++){
    // 直接为 levelDB 中的key 形式，直接进行查询操作即可
    // 先根据 key 获取到对应的 json value
    string parent_key = *item;
    string content = client.Get(parent_key);

    unordered_set<string> part_set;
    part_set.insert(parent_key);
    queue<string> related_queue;
    json value_json = json::parse(content);

    /*
    // 获取 boundingbox 相关信息
    std::ifstream file("/home/step192/step_data/boundingBox/assembly_boundingbox_S1712-5500001.json");
    json boundingbox_json;
    file >> boundingbox_json;
    file.close();

    string part_name = value_json["belong"];

    if (boundingbox_json.contains(part_name))
      leveldb::Status boundingbox_modified = dbJson->Put(leveldb::WriteOptions(), "boundingbox_" + parent_key, boundingbox_json[part_name]);
    */
    string belongAssembly = parent_key;

    string related_entity = value_json["related_entity"];
    vector<string> related_list =  split_parser(related_entity);
    // 获取到 MANIFOLD_SOLID_BREP 的 索引id，处理成 key 的形式便于后续查找 
    for (int i = 0; i < related_list.size(); i ++){
      string entity_key = fileName + leftPad(related_list[i], 16);
      related_queue.push(entity_key);
      part_set.insert(entity_key);
    }

    // ADVANCED_BREP_SHAPE_REPRESENTATION 处理完毕，写回 jsonDB 中
    while(!related_queue.empty()){
        string key = related_queue.front();
        related_queue.pop();
        
        string value = client.Get(key);
        //cout << "content = " << content << endl;
        json entity_json = json::parse(value);
        entity_json["belong"] = belongAssembly;   // 修改 belong 属性

        if (entity_json["related_entity"] != ""){  // 将其下属的实体索引 id 也置于队列中
          string entities = entity_json["related_entity"];
          //cout << "related_entity = " << entities << endl;
          vector<string> entity_list = split_parser(entities);

          for (int i = 0; i < entity_list.size(); i ++){ 
            string entity_key = fileName + leftPad(entity_list[i], 16);
            related_queue.push(entity_key);
            part_set.insert(entity_key);
          }
        }

        string modified_entity = entity_json.dump();
        bool entity_modified = client.Put(key, modified_entity);
        if (!entity_modified) {
          std::cout << "Error" << std::endl;
        }
    }
    
    string part_set_elements = serializeUnorderedSet(part_set);
    bool entity_modified = client.Put("part_" + parent_key, part_set_elements);
  }
}

std::vector<std::pair<std::unordered_set<std::string>::iterator, std::unordered_set<std::string>::iterator>> splitSet(
    std::unordered_set<std::string>& inputSet, size_t numThreads) {
    
    std::vector<std::pair<std::unordered_set<std::string>::iterator, std::unordered_set<std::string>::iterator>> result;
    auto begin = inputSet.begin();
    auto end = inputSet.end();
    size_t totalSize = std::distance(begin, end);

    if (totalSize == 0 || numThreads == 0) {
      cout << "该 STEP 文件没有子部件" << endl; 
      return result; // 如果输入集合为空或线程数为0，返回空结果
    }

    // 如果元素总数小于线程数，每个线程最多处理一个元素
    if (totalSize < numThreads) {
        for (size_t i = 0; i < totalSize; ++i) {
            result.emplace_back(begin++, begin); // 每个线程处理一个元素
        }
        // 处理多余的线程
        for (size_t i = totalSize; i < numThreads; ++i) {
            result.emplace_back(end, end); // 多余的线程不分配元素
        }
    } 
    else {
    // 否则，按照每个线程分配一块
    size_t chunkSize = totalSize / numThreads;
    for (size_t i = 0; i < numThreads; ++i) {
        auto nextBegin = std::next(begin, chunkSize);
        if (i == numThreads - 1) {
            nextBegin = end; // 最后一个线程获取剩余的元素
        }
        result.emplace_back(begin, nextBegin);
        begin = nextBegin;
        }
    }
    return result;
}

int step_analysis(LsmdbClient &client, const string& step_file) {
  // 统计 数据处理 + 存入levelDB中 的时间开销
  std::cout << "start analysis" << std::endl;
  auto start_leveldb = std::chrono::high_resolution_clock::now();
  
  // goal: 将 step 文件内容处理为 key-value 对进行存储
  // 将文件处理成字符串
  string stepFilePath = step_file;  
  size_t lastSlashPos = stepFilePath.find_last_of("/\\");
  if (lastSlashPos == std::string::npos) {
  // 如果找不到斜杠，则认为路径中没有目录部分，直接返回整个字符串
  lastSlashPos = 0;
  } else {
      // 否则从斜杠后的第一个字符开始提取文件名
      lastSlashPos += 1;
  }

  // 使用 substr 提取文件名
  std::string fileName = stepFilePath.substr(lastSlashPos);
  string stepContent = readStepFile(stepFilePath);

  // 找到文件名中最后一个点的位置
  size_t lastDotPos = fileName.find_last_of(".");
  if (lastDotPos != std::string::npos) {
      fileName = fileName.substr(0, lastDotPos);
  }

  // 将字符串内容根据 ";" 进行拆分形成 key-value 对
  vector<pair<string, string>> entities = parseStepEntity(stepContent);
  cout << fileName << endl;
  // 写入数据库
  vector<string> keys, values;
  for (int i = 0; i < entities.size(); i ++){
    auto [key, value] = entities[i];  
    // cout << key << " " << value << endl;
    // bool statusContent = client.Put(key, value);
    // if (!statusContent){
    //    throw runtime_error("无法将实体存储到LevelDB");
    // }
    keys.push_back(fileName + "_" + key);
    values.push_back(value);
  }
  bool statusContent = client.BatchPut(keys, values);
  if(statusContent)    cout << "put end" << endl;
  auto end_leveldb = std::chrono::high_resolution_clock::now();
  
  // 计算执行时长
  std::chrono::duration<double> duration_leveldb = end_leveldb - start_leveldb;
  std::cout << "处理 + 存入levelDB 执行时长: " << duration_leveldb.count() << " 秒" << std::endl;

  // 测试前缀查询功能
  std::string prefix = fileName; // 假设所有键都以 "prefix_" 开头
  std::vector<std::pair<std::string, std::string>> prefixData = client.GetDataWithPrefix(prefix);
  std::cout << "前缀查询结果：" << std::endl;
  for (const auto& kv : prefixData) {
      std::cout << "Key: " << kv.first << ", Value: " << kv.second << std::endl;
  }

//   leveldb::DB *dbJson;  // 定义数据库访问变量（声明相应的头文件）
//   leveldb::Options optionsJson;  // opstions定义，主要记录后续操作的相关设置
//   // optionsContent.create_if_missing = true;   // 设置为空，则新建数据库
//   optionsJson.create_if_missing = true;

//   // open jsondb
//   leveldb::Status statusJson = leveldb::DB::Open(optionsJson, "./jsondb", &dbJson);  // 选择打开的数据库，主要声明数据库的相应地址
//   assert(statusJson.ok());

  unordered_set<string> delete_id_json;
  unordered_set<string> part_entity_id;
  unordered_set<string> context_entity_id;
  // 定义解析的 switch-case
  setupAnalysis();

  int minKey = stoi(entities[0].first);
  int maxKey = stoi(entities[entities.size() - 1].first);

  int numThreads = 20;

  int keyWidth = 16;

  // 计算每个线程处理的 key 范围
  int keysPerThread = (maxKey - minKey + 1) / numThreads;  // 每个线程处理的 key 数量
  int remainderKeys = (maxKey - minKey + 1) % numThreads;  // 处理剩余的 key 数量

  // 存储每个线程处理的 key 范围
  vector<pair<string, string>> keyRanges;
  int currentKey = minKey;

  for (int i = 0; i < numThreads; i ++){
    int startKey = currentKey;
    int endKey = currentKey + keysPerThread - 1;
    if (i < remainderKeys) endKey += 1;
    currentKey = endKey + 1;

    // 迭代器中选用 '<'，故需要 endKey + 1，不包含在范围内
    keyRanges.emplace_back(leftPad(
      to_string(startKey), keyWidth), 
      leftPad(to_string(endKey + 1), keyWidth));
  }

  vector<std::thread> threads;

  for (auto range : keyRanges){
    threads.emplace_back(rangeQuery, std::ref(client), range.first, range.second, ref(delete_id_json), ref(part_entity_id), ref(context_entity_id),fileName);
  }

  // 等待所有线程完成
  for (auto& thread : threads) {
      thread.join();
  }

  // 范围查询：一次处理一段数据
  // status = db->Get(leveldb::ReadOptions(), key, &value);
  //assert(status.ok());

  // string retrieved_json;
  // statusJson = dbJson->Get(leveldb::ReadOptions(), "10", &retrieved_json);
  // if (statusJson.ok()){
  //   json retrieved_data = json::parse(retrieved_json);
  //   cout << "从 dbJson 数据库中获取到的数据： " << retrieved_data.dump(4) << endl;
  // }

  // 删除被压缩的实体对象
  /*
  for (auto id = delete_id_json.begin(); id != delete_id_json.end(); ++id) {
    string entity_id = fileName + *id;
    leveldb::Status delete_status = dbJson->Delete(leveldb::WriteOptions(), entity_id);
    if (!delete_status.ok()) {
        std::cerr << "Unable to delete key: " << delete_status.ToString() << std::endl;
    } 
  }
  */


  // auto splitDeleteId = splitSet(delete_id_json, numThreads);
  // vector<std::thread> delete_threads;

  // for (const auto& [start, end] : splitDeleteId){
  //   delete_threads.emplace_back(deleteEntity, start, end, fileName, dbJson);
  // }

  // for (auto& thread : delete_threads){
  //   thread.join();
  // }

  auto end_analysis = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double> duration_analysis = end_analysis - end_leveldb;
  std::cout << "解析生成 json 并存入levelDB 执行时长: " << duration_analysis.count() << " 秒" << std::endl;

//   leveldb::DB* db;
//   leveldb::Options options;
//   options.create_if_missing = false;

  std::vector<std::pair<std::string, std::string>> prefixData1 = client.GetDataWithPrefix("json");
  std::cout << "前缀查询结果：" << std::endl;
  for (const auto& kv : prefixData1) {
      std::cout << "Key: " << kv.first << ", Value: " << kv.second << std::endl;
  }
  cout << context_entity_id.size() << endl;
  for (auto context_id : context_entity_id){
    string context_content = client.Get(context_id);
    cout << context_content << endl;
    json value_json;// = json::parse(context_content);
    try {
        value_json = json::parse(context_content);
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON 解析错误: " << e.what() << std::endl;
        continue;  
    }
    string advanced_related_id = value_json["representation_relation_id"];
    advanced_related_id = fileName + leftPad(trim_brackets(advanced_related_id), 16);
    
    string advanced_related_content = client.Get(advanced_related_id);
    // cout << advanced_related_id << endl;
    // cout << advanced_related_content << endl;
    // cout << "============================" << endl;
    json value_advanced_related = json::parse(advanced_related_content);
    string advanced_brep_id = value_advanced_related["related_entity"];
    advanced_brep_id = fileName + leftPad(advanced_brep_id, 16);
    string advanced_brep_content = client.Get(advanced_brep_id);
    // cout << advanced_brep_id << endl;
    // cout << advanced_brep_content << endl;
    // cout << "-------------------------------------" << endl;
    json value_advanced_brep = json::parse(advanced_brep_content);
    if ( value_advanced_brep["entity_type"] == "ADVANCED_BREP_SHAPE_REPRESENTATION" ){
      string assembly_related_id = value_json["represented_product_relation_id"];
      assembly_related_id = fileName + leftPad(trim_brackets(assembly_related_id), 16);

      string assembly_related_content = client.Get(assembly_related_id);

      json value_assembly_related = json::parse(assembly_related_content);
      string assembly_id = value_assembly_related["related_entity"];
      assembly_id = fileName + leftPad(assembly_id, 16);
      
      string assembly_content = client.Get(assembly_id);

      json value_assembly = json::parse(assembly_content);

      string belong_assembly =  value_assembly["name"];
      value_advanced_brep["belong"] = trim_brackets(belong_assembly);
      string value_advanced = value_advanced_brep.dump();
      bool status_write = client.Put(advanced_brep_id, value_advanced);
      if (!status_write) 
        std::cerr << "数据存储失败" << std::endl;
    }
    
  }
  

//   leveldb::Status status = leveldb::DB::Open(options, "/tmp/jsondb", &db);

  
//   std::ofstream outFile("/home/step192/step_data/report/leveldb_json.txt");


//   auto splitPartId = splitSet(part_entity_id, numThreads);
//   vector<std::thread> split_part_threads;

//   for (const auto& [start, end] : splitPartId){
//     split_part_threads.emplace_back(splitPart, start, end, fileName, client);
//   }

//  for (auto& thread : split_part_threads){
//   thread.join();
//  }

//   auto end_analysis_part = std::chrono::high_resolution_clock::now();
//   std::chrono::duration<double> duration_analysis_part = end_analysis_part - end_analysis;
//   std::cout << "完成大型 STEP 文件的拆解耗时: " << duration_analysis_part.count() << " 秒" << std::endl;

  /*
  leveldb::ReadOptions readOptions_1;
  
  std::unique_ptr<leveldb::Iterator> it_1(dbJson->NewIterator(readOptions_1));
  for (it_1->SeekToFirst(); it_1->Valid(); it_1->Next()) {
      std::string key = it_1->key().ToString();
      std::string value = it_1->value().ToString();
      outFile << "Key: " << key << ", Value: " << value << std::endl;
  }

  outFile.close();
  */
  
  // close
  // std::cout << "关闭 client，完成存储。" << std::endl;
  // delete client;
  // std::cout << "关闭 dbJson，完成存储。" << std::endl;
  // delete dbJson;
  return 0;
}

// void step_store(const std::string& param, const std::string& db_name, const std::string& user, const std::string& password, const std::string& csv);

int main(int argc, char* argv[]){
    // LsmdbClient client(grpc::CreateChannel("localhost:9014", grpc::InsecureChannelCredentials()));
    // std::string key = "test_key";
    // std::string value = "test_value";
    // if (client.Put(key, value))
    //     std::cout << "写入成功：" << key << " -> " << value << std::endl;
    // std::string result = client.Get(key);
    // if (!result.empty())
    //     std::cout << "获取成功：" << key << " -> " << result << std::endl;
    // client.CloseDB();
    // return 0;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <stepFile>" << endl;
        return 1;
    }
    string stepFile = argv[1];
    std::string server_address = "localhost:9014";

    // 使用自定义的 ChannelArguments 创建 Channel
    std::shared_ptr<grpc::Channel> channel = CreateChannelWithMaxMessageSize(server_address);

    // 使用 channel 创建 LsmdbClient 实例
    LsmdbClient client(channel);
    // LsmdbClient client(grpc::CreateChannel("localhost:9014", grpc::InsecureChannelCredentials()));
    cout << "Analyzing step file: " << stepFile << endl;
    step_analysis(client, stepFile);
    return 0;
}
