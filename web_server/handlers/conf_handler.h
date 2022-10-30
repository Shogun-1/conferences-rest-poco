#ifndef CONFHANDLER_H
#define CONFHANDLER_H

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

#include "../../database/conf.h"

class ConfHandler : public HTTPRequestHandler
{
private:
    bool check_starttime(const std::string &starttime, std::string &reason)
    {
        if (starttime.length() != 19)
        {
            reason = "Starttime must be written as follows: YYYY-MM-DD HH:MM:SS";
            return false;
        }

        for (uint16_t i = 0; i < 19; i++)
        {
            if (i < 4 || (i > 4 && i < 7) || (i > 7 && i < 10) || (i > 10 && i < 13) || (i > 13 && i < 16) || i > 16)
            {
                if (!isdigit(starttime[i]))
                {
                    reason = "Starttime must be written as follows: YYYY-MM-DD HH:MM:SS";
                    return false;
                }
            }
        }

        return true;
    };

public:
    ConfHandler(const std::string &format) : _format(format)
    {
    }

    void handleRequest(HTTPServerRequest &request,
                       HTTPServerResponse &response)
    {
        HTMLForm form(request, request.stream());
        
        if (Poco::startsWith(request.getURI(), std::string("/conf/addpres")))
        {
            if (form.has("conf_title") && form.has("pres_title")) 
            {
                std::string conf_title = form.get("conf_title");
                std::string pres_title = form.get("pres_title");

                try
                {
                    long insert_id = database::Conf::add_pres(conf_title, pres_title);
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    std::ostream &ostr = response.send();
                    ostr << insert_id;
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

        }
        else if (Poco::startsWith(request.getURI(), std::string("/conf/addconf")))
        {
            if (form.has("conf_title") && form.has("starttime") && form.has("place"))
            {
                database::Conf conf;
                conf.title() = form.get("conf_title");
                conf.starttime() = form.get("starttime");
                conf.place() = form.get("place");

                std::string reason;
                if (!check_starttime(conf.get_starttime(), reason))
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                    std::ostream &ostr = response.send();
                    ostr << reason;
                    response.send();
                    return;
                }
            
                try
                {
                    conf.save_to_mysql();
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    std::ostream &ostr = response.send();
                    ostr << conf.get_id();
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
        }
        else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET && form.has("conf_title"))
        {
            try
            {
                std::string conf_title = form.get("conf_title");
                auto results = database::Conf::get_pres_list(conf_title);
                Poco::JSON::Array arr;
                for (auto s : results) {
                    Poco::JSON::Object::Ptr json = s.toJSON();
                    json->remove("id");
                    json->remove("theme");
                    json->remove("annotation");
                    json->remove("author");
                    json->remove("date");
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
        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        std::ostream &ostr = response.send();
        ostr << "request error";
        response.send();
    }

private:
    std::string _format;
};
#endif // !ConfHandler_H