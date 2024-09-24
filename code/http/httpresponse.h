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
    void MakeResponse(Buffer& buff); // �ú����л�ȷ��״̬�룬����private�е�������Ӻ���
    void UnmapFile();
    char* File();
    size_t FileLen() const;
    void ErrorContent(Buffer& buff, std::string message);
    int Code() const { return code_; }

private:
    void AddStateLine_(Buffer &buff); // ���״̬��
    void AddHeader_(Buffer &buff); // �����Ϣ��ͷ
    void AddContent_(Buffer &buff); // �����Ӧ����

    void ErrorHtml_();
    std::string GetFileType_();

    int code_; // ״̬��
    bool isKeepAlive_; // http�������

    /*����״̬��*/
    std::string path_;
    std::string srcDir_; // ԴĿ¼
    
    char* mmFile_;  // ӳ���ļ�
    struct stat mmFileStat_; // �ļ�״̬��Ϣ

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE; // ��׺����
    static const std::unordered_map<int, std::string> CODE_STATUS; // http״̬��
    static const std::unordered_map<int, std::string> CODE_PATH; // 400��403��404��Ӧ�Ĵ���·��
};


#endif //HTTP_RESPONSE_H