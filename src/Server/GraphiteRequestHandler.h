#pragma once

#include <IO/HTTPCommon.h>
#include <Server/HTTP/WriteBufferFromHTTPServerResponse.h>
#include <Server/HTTPHandlerFactory.h>
#include <Server/IServer.h>
#include <Common/CurrentMetrics.h>
#include <Common/Exception.h>
#include <Common/ProfileEvents.h>

#include <Poco/Util/LayeredConfiguration.h>

#include <optional>
#include <Core/Names.h>
#include <Server/HTTP/HTMLForm.h>
#include <Server/HTTP/HTTPRequestHandler.h>
#include <Server/HTTP/WriteBufferFromHTTPServerResponse.h>
#include <Server/HTTPHandler.h>
#include <Common/CurrentMetrics.h>
#include <Common/CurrentThread.h>


namespace DB
{

class IServer;

class GraphiteRequestHandler : public HTTPHandler
{
private:
    IServer & server;
    Poco::Logger * log = &Poco::Logger::get("GraphiteCarbon");
    std::string table_name;

public:
    explicit GraphiteRequestHandler(
        IServer & server_,
        const std::string & table_name_ = "graphite_index",
        const std::optional<String> & content_type_override_ = std::nullopt)
        : HTTPHandler(server_, "graphite_carbon", content_type_override_), server(server_), table_name(table_name_)
    {
    }
    std::string getQuery(HTTPServerRequest & request, HTMLForm & params, ContextMutablePtr context) override;

    bool customizeQueryParam(ContextMutablePtr context, const std::string & key, const std::string & value) override;
};

}
