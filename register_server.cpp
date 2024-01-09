#include <iostream>
#include <sstream>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

#include "sql/sql_warpper.h"

const int max_register_count = 6;
const int PORT = 7790;

void send_response(int socket, const std::string &body) {
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n"
        << "Content-Type: text/plain\r\n"
        << "Content-Length: " << body.length() << "\r\n"
        << "\r\n"
        << body;
    std::string response = oss.str();
    write(socket, response.c_str(), response.length());
}

int main() {

    // 构建数据库
    sql_warpper sql_("tcp://127.0.0.1:3306", "root", "Windows77", "game");
    // 构建格
    sql_.create("create table cloud_register_server_data ("
			"register_count BIGINT, rsa_code TEXT NOT NULL);");

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    char buffer[30000] = {0};

    // 创建socket文件描述符
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    memset(address.sin_zero, '\0', sizeof address.sin_zero);

    // 绑定socket到端口8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("In bind");
        exit(EXIT_FAILURE);
    }

    // 监听请求
    if (listen(server_fd, 10) < 0) {
        perror("In listen");
        exit(EXIT_FAILURE);
    }

    while (true) {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

        long valread = read(new_socket, buffer, 30000);
        
        // 检查是否是POST请求
        if (strncmp(buffer, "POST ", 5) == 0) {
            char *body = strstr(buffer, "\r\n\r\n");
            if (body) {
                body += 4;  // 跳过"\r\n\r\n"
                std::cout << "POST body: " << body << std::endl;
				
				// 检测body
				std::string content = body;		

				// 查询
				std::vector<std::tuple<int, std::string>> register_content_;
				// 查询
				auto ret_ = sql_.select("select * from cloud_register_server_data;",
					std::tuple <>(), std::tuple<std::string, std::string>("register_count", "rsa_code"),
					register_content_);

				bool registered = false;
				for (auto& val : register_content_)
				{
					std::size_t register_count = std::get<0>(val);
					std::string rsa_code = std::get<1>(val);

					// 已经注册
					if (content == rsa_code)
					{

						std::cout << "register_count " << register_count << "\n";
						// 更新注册次数
						registered = true;
						if (0 < register_count)
						{
							std::cout << "register_count " << register_count << "\n";

							// 更新次数
							std::string cmd = "update cloud_register_server_data set register_count = ? where rsa_code = ?;";
							ret_ = sql_.update(cmd, std::tuple<int, std::string>(register_count - 1, rsa_code));

							std::cout << "ret " << ret_ << "\n";

                            std::string real_response;
                            if (ret_)
							    real_response = "{ \"tag\": \"response\", \"value\": \"yes\"  }";
                            else
                         		// 返回失败
							    real_response = "{ \"tag\": \"response\", \"value\": \"no\", \"reason\": \"invalid sql\" }";
                            
                            send_response(new_socket, real_response);       
							// 返回成功
							std::cout << "ret value = " << real_response;
						}
						else
						{
							// 返回失败
							std::string real_response = "{ \"tag\": \"response\", \"value\": \"no\", \"reason\": \"invalid count\" }";
							send_response(new_socket, real_response);
							
							// 返回失败
							std::cout << "invalid count " << real_response;
						}
						break;
					}
				}

				// 尚未注册
				if (!registered)
				{
					// 添加 —> 带参数
					ret_ = sql_.insert("insert into cloud_register_server_data "
					   " (register_count, rsa_code) values (?, ?);",
						std::tuple<int, std::string>(max_register_count - 1, content));

                    std::string real_response;
                    if (ret_)
					    real_response = "{ \"tag\": \"response\", \"value\": \"yes\"  }";
                    else
					    // 返回失败
						real_response = "{ \"tag\": \"response\", \"value\": \"no\", \"reason\": \"invalid sql\" }";
                    send_response(new_socket, real_response);
					// 返回成功
					std::cout << "ret value = " << real_response;
                }
            }
        }
        // 关闭连接
        close(new_socket);
    }
    return 0;
}

