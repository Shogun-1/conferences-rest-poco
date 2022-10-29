#ifndef PRESHANDLER_H
#define PRESHANDLER_H

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

#include "../../database/pres.h"

class PresHandler : public HTTPRequestHandler
{
private:
    bool check_name(const std::string &name, std::string &reason)
    {
        if (name.length() < 3)
        {
            reason = "Name must be at least 3 letters long";
            return false;
        }

        int num_of_spaces = 0;
        for (const char& ch : name) 
        {
            if (ch == ' ') 
            {
                num_of_spaces++;
            }
        }
        if (num_of_spaces != 1)
        {
            reason = "Name must contain author's first name and last name separated by space";
            return false;
        }

        if (name.find('\t') != std::string::npos)
        {
            reason = "Name can't contain tabs";
            return false;
        }

        return true;
    };

    bool check_date(const std::string &date, std::string &reason)
    {
        if (date.length() != 10)
        {
            reason = "Date must be written as follows: YYYY-MM-DD";
            return false;
        }

        for (uint16_t i = 0; i < 10; i++)
        {
            if (i < 4 || (i > 4 && i < 7) || (i > 7))
            {
                if (!isdigit(date[i]))
                {
                    reason = "Date must be written as follows: YYYY-MM-DD";
                    return false;
                }
            }
        }

        return true;
    };

public:
    PresHandler(const std::string &format) : _format(format)
    {
    }

    void handleRequest(HTTPServerRequest &request,
                       HTTPServerResponse &response)
    {
        HTMLForm form(request, request.stream());
        if (Poco::startsWith(request.getURI(), std::string("/pres/getall")))
        {
            try
            {
                auto results = database::Pres::get_all();
                Poco::JSON::Array arr;
                for (auto s : results) {
                    Poco::JSON::Object::Ptr json = s.toJSON();
                    json->remove("id");
                    arr.add(json);
                }
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(arr, ostr);
            }
            catch (const std::exception &exc)
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
            if (form.has("title") && form.has("theme") && form.has("annotation") && form.has("author") && form.has("date"))
            {
                database::Pres pres;
                pres.title() = form.get("title");
                pres.theme() = form.get("theme");
                pres.annotation() = form.get("annotation");
                pres.author() = form.get("author");
                pres.date() = form.get("date");

                bool check_result = true;
                std::string message;
                std::string reason;

                if (!check_name(pres.get_author(), reason))
                {
                    check_result = false;
                    message += reason;
                    message += "<br>";
                }

                if (!check_date(pres.get_date(), reason))
                {
                    check_result = false;
                    message += reason;
                    message += "<br>";
                }

                if (check_result)
                {
                    try
                    {
                        pres.save_to_mysql();
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        std::ostream &ostr = response.send();
                        ostr << pres.get_id();
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
#endif // !PresHandler_H