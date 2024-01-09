#ifndef ASIOHTTPSERVER_H
#define ASIOHTTPSERVER_H

#include <asio2/http/http_server.hpp>
#include <memory>
#include <string_view>

//#include "sql/sql_warpper.h"

enum 
{
    max_register_count = 6,
};

class AsioHttpServer
{
public:
    AsioHttpServer() {
		// 构建数据库
  //      sql_warpper sql_("tcp://127.0.0.1:3306", "root", "Windows77", "game");
		//// 构建表格
		//sql_.create("create table magic_keys_register_server_data ("
		//	"register_count BIGINT, rsa_code TEXT NOT NULL);");
    }
    bool mbStartServer()
    {
        std::string_view host = "0.0.0.0";
        std::string_view port = "7789";

        // 设置路径
        std::filesystem::path root = std::filesystem::current_path().parent_path().parent_path().append("wwwroot");
        server_.set_root_directory(std::move(root));

        // 绑定路径
        server_.bind<http::verb::get, http::verb::post>("/", 
            [this](http::web_request& req, http::web_response& rep)
        {
            asio2::ignore_unused(req, rep);
            auto body = req.body();

            // 查询
			//sql_warpper sql_("tcp://127.0.0.1:3306", "root", "Windows77", "game");
			std::vector<std::tuple<int, std::string>> register_content_;
			// 查询
			//auto ret_ = sql_.select("select * from magic_keys_register_server_data;",
			//	std::tuple <>(), std::tuple<std::string, std::string>("register_count", "rsa_code"),
   //             register_content_);

            bool registered = false;
			for (auto& val : register_content_)
			{
                std::size_t register_count = std::get<0>(val);
                std::string rsa_code = std::get<1>(val);

                // 已经注册
                if (body == rsa_code)
                {
					// 更新注册次数
					registered = true;
                    if (0 < register_count)
                    {
						// 更新次数
						//std::string cmd = "update magic_keys_register_server_data set register_count = " + std::to_string(register_count - 1)
						//	+ " where rsa_code = " + rsa_code + ";";
						//ret_ = sql_.update(cmd);
						// 返回成功
						rep.fill_text("{ \"tag\": \"response\", \"value\": \"yes\"  }");
                    }
                    else
                    {
						// 返回成功
						rep.fill_text("{ \"tag\": \"response\", \"value\": \"no\", \"reason\": \"invalid count\" }");
                    }
                    break;
                }
			}

            // 尚未注册
            if (!registered)
            {
				// 添加 —> 带参数
				//ret_ = sql_.insert("insert into magic_keys_register_server_data "
    //               " (register_count, rsa_code) values (?, ?);",
    //                std::tuple<std::size_t, std::string>(max_register_count - 1, body));
                // 返回成功
                rep.fill_text("{ \"tag\": \"response\", \"value\": \"yes\"  }");
            }
            rep.chunked(true);
        });

        // 启动服务器
        server_.start(host, port);
        return true;
    }
    bool mbStopServer()
    {
        server_.stop();
        return true;
    }

private:
    asio2::http_server server_;
};

#endif // ASIOHTTPSERVER_H
