#include "CloudiskServer.h"
#include "Token.h"
#include "Hash.h"
#include "unixHeader.h"

#include <workflow/MySQLMessage.h>
#include <workflow/MySQLResult.h>
#include <wfrest/json.hpp>

#include <iostream>

using namespace wfrest;
using std::string;
using std::cout;
using std::endl;
using std::vector;

void CloudiskServer::start(unsigned short port)
{
    if(_httpserver.track().start(port) == 0) {
        _httpserver.list_routes();
        _waitGroup.wait();
        _httpserver.stop();
    } else {
        printf("Cloudisk Server Start Failed!\n");
    }
}

void CloudiskServer::loadModules()
{
    loadStaticResourceModule();
    //loadUserRegisterModule();
    //loadUserLoginModule();
    //loadUserInfoModule();
    //loadFileQueryModule();
    //loadFileUploadModule();
    //loadFileDownloadModule();
}

void CloudiskServer::loadStaticResourceModule()
{
    _httpserver.GET("/user/signup", [](const HttpReq *, HttpResp * resp){
        resp->File("static/view/signup.html");
    });

    _httpserver.GET("/static/view/signin.html", [](const HttpReq *, HttpResp * resp){
        resp->File("static/view/signin.html");
    });
    
    _httpserver.GET("/static/view/home.html", [](const HttpReq *, HttpResp * resp){
        resp->File("static/view/home.html");
    });

    _httpserver.GET("/static/js/auth.js", [](const HttpReq *, HttpResp * resp){
        resp->File("static/js/auth.js");
    });

    _httpserver.GET("/static/img/avatar.jpeg", [](const HttpReq *, HttpResp * resp){
        resp->File("static/img/avatar.jpeg");
    });

    _httpserver.GET("/file/upload", [](const HttpReq *, HttpResp * resp){
        resp->File("static/view/index.html");
    });
    _httpserver.Static("/file/upload_files","static/view/upload_files");
}

