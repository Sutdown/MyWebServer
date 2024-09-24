#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <unordered_map>
#include <fcntl.h>       // open
#include <unistd.h>      // close
#include <sys/stat.h>    // stat
#include <sys/mman.h>    // mmap, munmap

#include "../buffer/buffer.h"
#include "../log/log.h"

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1);
    void MakeResponse(Buffer& buff); // 该函数中会确定状态码，调用private中的三个添加函数
    void UnmapFile();
    char* File();
    size_t FileLen() const;
    void ErrorContent(Buffer& buff, std::string message);
    int Code() const { return code_; }

private:
    void AddStateLine_(Buffer &buff); // 添加状态行
    void AddHeader_(Buffer &buff); // 添加消息报头
    void AddContent_(Buffer &buff); // 添加响应正文

    void ErrorHtml_();
    std::string GetFileType_();

    int code_; // 状态码
    bool isKeepAlive_; // http保活机制

    /*生成状态码*/
    std::string path_;
    std::string srcDir_; // 源目录
    
    char* mmFile_;  // 映射文件
    struct stat mmFileStat_; // 文件状态信息

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE; // 后缀类型
    static const std::unordered_map<int, std::string> CODE_STATUS; // http状态码
    static const std::unordered_map<int, std::string> CODE_PATH; // 400，403，404对应的错误路径
};


#endif //HTTP_RESPONSE_H