#include "Mime.h"
#include "config.h"
#include <iostream>

using namespace std;
void MimeType::init(){    
            mime = {
                {".html", "text/html"},
                {".xml", "text/xml"},
                {".xhtml", "application/xhtml+xml"},
                {".txt", "text/plain"},
                {".rtf", "application/rtf"},
                {".pdf", "application/pdf"},
                {".word", "application/nsword"},
                {".png", "image/png"},
                {".gif", "image/gif"},
                {".jpg", "image/jpeg"},
                {".jpeg", "image/jpeg"},
                {".au", "audio/basic"},
                {".mpeg", "video/mpeg"},
                {".mpg", "video/mpeg"},
                {".avi", "video/x-msvideo"},
                {".gz", "application/x-gzip"},
                {".tar", "application/x-tar"},
                {".css", "text/css "},
                {".js", "text/javascript "},
                {"", "text/html"},
                {"default", "text/html"}
            };
#ifdef _WEB_
    cout << mime.size() << endl;
#endif
}

std::string MimeType::get_mime(std::string suffix)
{
    if (mime.find(suffix) == mime.end())
    {
        return "text/html";
    }
    else
    {
        return mime[suffix];
    }
}


std::string WebPage::get_page(std::string _fileName){
    if(filesMap.find(_fileName) != filesMap.end()){
        return filesMap[_fileName];
    }else{
        return "error.html";
    }
}

void  WebPage::init(){
       if(filesMap.size() == 0){
            filesMap = {
                {"index", "index.html"},
                {"picture", "picture.html"},
                {"vedio", "vedio.html"},
                {"login", "login.html"},
                {"welcome", "welcome.html"},
                {"register", "register.html"},
                {"error", "error.html",},
                {"", "index.html"}
            };
        }
#ifdef _WEB_
    cout << filesMap.size() << endl;
#endif
}

