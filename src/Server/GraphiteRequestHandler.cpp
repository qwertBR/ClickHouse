#include <Server/GraphiteRequestHandler.h>


#include <IO/HTTPCommon.h>
#include <Server/GraphiteFinder.h>
#include <Server/GraphiteRender.h>
#include <Server/HTTP/WriteBufferFromHTTPServerResponse.h>
#include <Server/HTTPHandlerFactory.h>
#include <Server/IServer.h>
#include <Common/CurrentMetrics.h>
#include <Common/Exception.h>
#include <Common/ProfileEvents.h>

#include <Poco/Util/LayeredConfiguration.h>


namespace DB
{

class IServer;

std::string GraphiteRequestHandler::getQuery(HTTPServerRequest & request, HTMLForm & params, ContextMutablePtr context)
{
    Poco::URI uri{request.getURI()};
    if (uri.toString().find("/metrics/find?") != std::string::npos)
    {
        LOG_DEBUG(log, "METRICS: {}", uri.toString());
        LOG_DEBUG(log, "table name: {}", table_name);
        const auto & t = server.config().getString("graphite_carbon.graphite_prefix.prefix", "prefix");
        LOG_DEBUG(log, "prefix: {}", t);
        std::string q;
        std::string format = "TabSeparatedRaw";
        int from = 0;
        int until = 0;
        for (auto el : uri.getQueryParameters())
        {
            LOG_DEBUG(log, "METRICS PARAM: {} : {}", el.first, el.second);
            if (el.first == "query")
            {
                q = el.second;
            }
            else if (el.first == "format")
            {
                format = el.second;
            }
            else if (el.first == "from")
            {
                from = GraphiteCarbon::IntervalStrings(el.second);
            }
            else if (el.first == "until")
            {
                until = GraphiteCarbon::IntervalStrings(el.second);
            }
        }
        for (const auto & it : params)
        {
            customizeQueryParam(context, it.first, it.second);
        }
        return GraphiteCarbon::MetricsFind(server, table_name, q, from, until, format);
    }
    else if (uri.toString().find("/render?") != std::string::npos)
    {
        LOG_DEBUG(log, "RENDER: {}", uri.toString());
        std::string target = "";
        std::string format = "RowBinary";
        int from = 0;
        int until = 0;
        for (auto el : uri.getQueryParameters())
        {
            LOG_DEBUG(log, "RENDER PARAM: {} : {}", el.first, el.second);
            if (el.first == "target")
            {
                target = el.second;
            }
            else if (el.first == "from")
            {
                from = GraphiteCarbon::IntervalStrings(el.second);
                LOG_DEBUG(log, "from: {}", from);
            }
            else if (el.first == "format")
            {
                format = el.second;
            }
            else if (el.first == "until")
            {
                until = GraphiteCarbon::IntervalStrings(el.second);
                LOG_DEBUG(log, "until: {}", until);
            }
        }
        return GraphiteCarbon::RenderQuery(server, table_name, target, from, until, format);
    }
    if (!context)
    {
    }
    return "";
}


bool GraphiteRequestHandler::customizeQueryParam(ContextMutablePtr context, const std::string & key, const std::string & value)
{
    if (!context)
    {
        return false;
    }
    if (key == "until" || key == "from" || key == "target" || key == "format" || key == "query")
    {
        return true;
    }
    if (value == "")
    {
        return false;
    }
    return false;
}


HTTPRequestHandlerFactoryPtr createGraphiteHandlerFactory(IServer & server, const std::string & config_prefix)
{
    const auto & table_name = server.config().getString(config_prefix + ".table_name", "graphite_index");
    std::optional<String> content_type_override;
    if (server.config().has(config_prefix + ".handler.content_type"))
        content_type_override = server.config().getString(config_prefix + ".handler.content_type");

    auto factory = std::make_shared<HandlingRuleHTTPHandlerFactory<GraphiteRequestHandler>>(
        server, std::move(table_name), std::move(content_type_override));

    factory->addFiltersFromConfig(server.config(), config_prefix);

    return factory;
}

}
