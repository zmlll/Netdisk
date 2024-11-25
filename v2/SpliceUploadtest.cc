#include <signal.h>
#include <workflow/WFFacilities.h>
#include <workflow/WFHttpServer.h>
#include <workflow/HttpUtil.h>
#include <wfrest/json.hpp>
#include "unixHeader.h"

using Json = nlohmann::json;
static WFFacilities::WaitGroup waitGroup(1);

void handler(int signum)
{
    printf("finsh 1 work!\n");
    waitGroup.done();
}

void process(WFHttpTask *serverTask)
{
    auto req = serverTask->get_req();
    auto resp = serverTask->get_resp();
    std::string uri = req->get_request_uri();
    std::string path = uri.substr(0,uri.find("?"));
    std::string query = uri.substr(uri.find("?")+1);
    std::string method = req->get_method();
    if(path == "/file/mupload/init" && method == "POST")
    {
        //1.解析请求报文
        const void* body;
        size_t size;
        req->get_parsed_body(&body,&size);

        //2.从字符串解析成json
        Json fileInfo = Json::parse(static_cast<const char* >(body));
        std::string filename = fileInfo["filename"];
        int filesize = fileInfo["filesize"];
        std::string filehash = fileInfo["file_hash"];

        //3.初始化分块操作
        //生成uploadID
        std::string uploadID = "zml";
        time_t now = time(NULL);
        struct tm *ptm = localtime(&now);
        char timeStamp[30] = {0};
        sprintf(timeStamp,"%02d%02d%02d", ptm->tm_hour,ptm->tm_min,ptm->tm_sec);


        //生成分块信息
        int chunkcount;
        int chunksize = 1024*1024;
        if(filesize % chunksize != 0)
        {
            chunkcount = filesize/chunksize +1;
        }
        else
        {
            chunkcount = filesize/chunksize;
        }
        
        //fprintf(stderr,"chunksize = %d, chunkcount = %d\n", chunksize, chunkcount);
        // 初始化redis
        // HSET uploadID chunkcount 6
        // HSET uploadID filehash xxx
        // HSET uploadID filesize xxx
        
        std::vector<std::vector<std::string>> argsVec = 
        {
            {uploadID,"chunkcount",std::to_string(chunkcount)},
            {uploadID,"filehash",filehash},
            {uploadID,"filesize",std::to_string(filesize)}
        };

        for(int i = 0;i <3;++i)
        {
            auto redisTask = WFTaskFactory::create_redis_task("redis://userzml:zml0201%@localhost",0,nullptr);
            redisTask->get_req()->set_request("HSET",argsVec[i]);
            redisTask->start();
        }
        // 生成响应给客户端
        Json uppartInfo;
        uppartInfo["status"] = "OK";
        uppartInfo["uploadID"] = uploadID;
        uppartInfo["chunkcount"] = chunkcount;
        uppartInfo["filesize"] = filesize;
        uppartInfo["chunksize"] = chunksize;
        uppartInfo["filehash"] = filehash;
        resp->append_output_body(uppartInfo.dump());
    }
    else if(path == "/file/mupload/uppart" && method == "POST")
    {
       //解析uri的查询参数，提取出uploadID和chkidx
       //query = "uploadID = zml123&chkidx = 2"
        std::string uploadIDKV = query.substr(0,query.find("&"));
        std::string uploadID = uploadIDKV.substr(uploadIDKV.find("=")+1);
        std::string chkidxKV = query.substr(query.find("&")+1);
        std::string chkidx = chkidxKV.substr(chkidxKV.find("=")+1);

        //保存单个分片 ./filehash/1 filehash需要从redis中获取 HGET uploadID filehash
        auto redisTaskHGET = WFTaskFactory::create_redis_task("redis://localhost",0,
        [chkidx,req](WFRedisTask *redisTaskHGET)
        {
          protocol::RedisValue value;
          redisTaskHGET->get_resp()->get_result(value);
          std::string filehash = value.string_value();
          mkdir(filehash.c_str(),0777);
          std::string filepath = filehash + "/" + chkidx;

          //文件IO任务
          int fd = open(filepath.c_str(),O_RDWR|O_CREAT,0666);
          const void* body;
          size_t size;
          req->get_parsed_body(&body,&size);
          write(fd,body,size);
          close(fd);
        });
        redisTaskHGET->get_req()->set_request("HGET",{uploadID,"filehash"});
        series_of(serverTask)->push_back(redisTaskHGET);
        //记录下载进度 HSET uploadID chkidx 1
        auto redisTaskHSET = 
            WFTaskFactory::create_redis_task("redis://localhost",0,nullptr);
        redisTaskHSET->get_req()->set_request("HSET",
            {uploadID,"chkidx_"+chkidx,"1"});
        series_of(serverTask)->push_back(redisTaskHSET);

        //设置响应的内容
        resp->append_output_body("OK");
    }
    else if(path == "/file/mupload/complete" && method == "POST")
    {
        //fprintf(stderr,"complete\n");
        //解析请求，提取出uploadID
        std::string uploadID = query.substr(query.find("=")+1);
        //查询上传进度
        // HGETALL uploadID
        // chunkcount键所对应的值-->下载的总进度
        // 前缀为chkidx_的键的数量-->下载的当前进度
        auto redisTask = WFTaskFactory::create_redis_task("redis://localhost",0,
            [resp](WFRedisTask *redistask)
            {
                protocol::RedisValue value;
                redisTask->get_resp()->get_result();
                int chunkcount;
                int chunknow = 0;
                for(int i =0;i<value.arr_size();i+=2)
                {
                    std::string key = value.arr_at(i).string_value();
                    std::string val = value.arr_at(i+1).string_value();
                    if(key == "chunkcount")
                    {
                        chunkcount = std::stoi(val);
                    }
                    else if(key.substr(0,7) == "chkidx_")
                    {
                        ++chunknow;
                    }
                }
                if(chunkcount == chunknow)
                {
                    resp->append_output_body("SUCCESS");
                }
                else
                {
                    resp->append_output_body("FAIL");
                }
            });
            redisTask->get_req()->set_request("HGETALL",{uploadID});
            series_of(serverTask)->push_back(redisTask);
    }

}


int main()
{
    signal(SIGINT,handler);
    WFHttpServer server(process);
    if(server.start(1234)==0)
    {
        waitGroup.wait();
        server.stop();
    }
    else
    {
        perror("server start error!\n");
        exit(-1);
    }
    return 0;
}
