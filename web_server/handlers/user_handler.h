#ifndef USERHANDLER_H
#define USERHANDLER_H

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/String.h"
#include <iostream>
#include <fstream>

using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTMLForm;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::NameValueCollection;
using Poco::Net::ServerSocket;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;

#include "../../database/user.h"

class UserHandler : public HTTPRequestHandler
{
private:
    bool check_name(const std::string &name, std::string &reason)
    {
        if (name.length() < 3)
        {
            reason = "Name must be at least 3 signs";
            return false;
        }

        if (name.find(' ') != std::string::npos)
        {
            reason = "Name can't contain spaces";
            return false;
        }

        if (name.find('\t') != std::string::npos)
        {
            reason = "Name can't contain spaces";
            return false;
        }

        return true;
    };

    bool check_email(const std::string &email, std::string &reason)
    {
        if (email.find('@') == std::string::npos)
        {
            reason = "Email must contain @";
            return false;
        }

        if (email.find(' ') != std::string::npos)
        {
            reason = "EMail can't contain spaces";
            return false;
        }

        if (email.find('\t') != std::string::npos)
        {
            reason = "EMail can't contain spaces";
            return false;
        }

        return true;
    };

public:
    UserHandler(const std::string &format) : _format(format)
    {
    }

    void handleRequest(HTTPServerRequest &request,
                       HTTPServerResponse &response)
    {
        HTMLForm form(request, request.stream());
        if (Poco::startsWith(request.getURI(), std::string("/user/get")) && form.has("login"))
        {
            try
            {
                std::string login = form.get("login");
                bool use_cache = true;
                if (form.has("no_cache")) {
                    if (form.get("no_cache") == "true") {
                        use_cache = false;
                    }
                }

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();

                if (use_cache) 
                {
                    try
                    {
                        std::optional<database::User> result = database::User::read_from_cache_by_login(login);
                        if (result) {
                            Poco::JSON::Object::Ptr json = result->toJSON();
                            json->remove("id");
                            json->remove("login");
                            json->remove("password");
                            Poco::JSON::Stringifier::stringify(json, ostr);
                            std::cout << "Cache used to fetch data for user " << login << std::endl;
                            return;
                        }
                    }
                    catch (...)
                    {
                        std::cout << "Cache missed to fetch data for user " << login << std::endl;
                    }
                }

                database::User result = database::User::read_by_login(login);
                // Saving the user entry to cache in case of a cache miss (if cache is enabled)
                if (use_cache) {
                    result.save_to_cache();
                    std::cout << "Added the entry to cache for user " << login << std::endl;
                }
                Poco::JSON::Object::Ptr json = result.toJSON();
                json->remove("id");
                json->remove("login");
                json->remove("password");
                Poco::JSON::Stringifier::stringify(json, ostr);
                return;
            }
            catch (...)
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                std::ostream &ostr = response.send();
                ostr << "{ \"result\": false , \"reason\": \"not found\" }";
                response.send();
                return;
            }
        }
        else if (Poco::startsWith(request.getURI(), std::string("/user/search")) && form.has("first_name") && form.has("last_name"))
        {
            try
            {
                
                std::string fn = form.get("first_name");
                std::string ln = form.get("last_name");
                auto results = database::User::search(fn, ln);
                Poco::JSON::Array arr;
                for (auto s : results) {
                    Poco::JSON::Object::Ptr json = s.toJSON();
                    json->remove("id");
                    json->remove("password");
                    arr.add(json);
                }
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(arr, ostr);
            }
            catch (...)
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                std::ostream &ostr = response.send();
                ostr << "{ \"result\": false , \"reason\": \"not found\" }";
                response.send();
                return;
            }
            return;
        }
        else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
        {
            if (form.has("login") && form.has("password") && form.has("first_name") && form.has("last_name") && form.has("email") and form.has("title"))
            {
                bool use_cache = true;
                if (form.has("no_cache")) {
                    if (form.get("no_cache") == "true") {
                        use_cache = false;
                    }
                }

                database::User User;
                User.login() = form.get("login");
                User.password() = form.get("password");
                User.first_name() = form.get("first_name");
                User.last_name() = form.get("last_name");
                User.email() = form.get("email");
                User.title() = form.get("title");

                bool check_result = true;
                std::string message;
                std::string reason;

                if (!check_name(User.get_first_name(), reason))
                {
                    check_result = false;
                    message += reason;
                    message += "<br>";
                }

                if (!check_name(User.get_last_name(), reason))
                {
                    check_result = false;
                    message += reason;
                    message += "<br>";
                }

                if (!check_email(User.get_email(), reason))
                {
                    check_result = false;
                    message += reason;
                    message += "<br>";
                }

                if (check_result)
                {
                    try
                    {
                        // Saving data both to the database and cache (if cache is enabled)
                        User.save_to_mysql();
                        if (use_cache) {
                            User.save_to_cache();
                            std::cout << "Saved user " + User.login() + " both to the database and redis cache." << std::endl;
                        } else {
                            std::cout << "Saved user " + User.login() + " to the database only." << std::endl;
                        }
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        std::ostream &ostr = response.send();
                        ostr << User.get_id();
                        return;
                    }
                    catch (...)
                    {
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                        std::ostream &ostr = response.send();
                        ostr << "database error";
                        response.send();
                        return;
                    }
                }
                else
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                    std::ostream &ostr = response.send();
                    ostr << message;
                    response.send();
                    return;
                }
            }
        }
        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        std::ostream &ostr = response.send();
        ostr << "request error";
        response.send();
    }

private:
    std::string _format;
};
#endif // !UserHandler_H