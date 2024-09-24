 #ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>     
#include <mysql/mysql.h>  //mysql

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnpool.h"
#include "../pool/sqlconnRAII.h"

class HttpRequest {
public:
    enum PARSE_STATE {
        REQUEST_LINE, // 请求行
        HEADERS, // 请求头
        BODY, // 请求体
        FINISH,        
    };

    enum HTTP_CODE {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };
    
    HttpRequest() { Init(); }
    ~HttpRequest() = default;

    void Init();
    bool parse(Buffer& buff); // 解析缓冲区

    std::string path() const;
    std::string& path();
    std::string method() const;
    std::string version() const;
    std::string GetPost(const std::string& key) const;
    std::string GetPost(const char* key) const;

    bool IsKeepAlive() const;

    /* 
    todo 
    void HttpConn::ParseFormData() {}
    void HttpConn::ParseJson() {}
    */

private:
    /*请求报文：
    * 请求行：method_ path_ version_
    * 请求头：host,user-agent,connection,...
    * 空行
    * 请求数据：get请求数据为空，post有请求体
    */
    bool ParseRequestLine_(const std::string& line); // 处理请求行
    void ParseHeader_(const std::string& line); // 处理请求头
    void ParseBody_(const std::string& line); // 处理请求体

    void ParsePath_(); // 处理请求路径
    void ParsePost_(); // 处理post事件
    void ParseFromUrlencoded_(); // 从url中解析编码

    static bool UserVerify(const std::string& name, const std::string& pwd, bool isLogin); // 用户验证

    PARSE_STATE state_;
    std::string method_, path_, version_, body_;
    std::unordered_map<std::string, std::string> header_;
    std::unordered_map<std::string, std::string> post_;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
    static int ConverHex(char ch); // 16进制转十进制
};


#endif //HTTP_REQUEST_H