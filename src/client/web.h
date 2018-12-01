#ifndef CLIENTWEB_H_
#define CLIENTWEB_H_

#include <atomic>
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <core/net/http/request.h>
#include <core/net/uri.h>

#include <QXmlStreamReader>

#include "types.h"

namespace client {

/**
 * Provide a nice way to access the HTTP API.
 *
 * We don't want our scope's code to be mixed together with HTTP and JSON handling.
 */
class Web {
public:



    /**
     * Result struct
     */
    struct Result {
        std::string uri;
        std::string title;
        std::string art;
        std::string subtitle;
        std::string description;
    };

    /**
     * A list of weather information
     */
    typedef std::deque<Result> ResultList;

    Web(config::Base::Ptr config);

    virtual ~Web() = default;

    /**
     * Search for results
     */
    virtual QString get_favicon_url(std::string url, bool redirected_url = false);
    virtual QString get_source_url_from_feedly(std::string src_url, std::string redirected_url = "");
    /**
     * Cancel any pending queries (this method can be called from a different thread)
     */
    virtual void cancel();

    virtual config::Base::Ptr config();

public:
    void get(const core::net::Uri::Path &path,
             const core::net::Uri::QueryParameters &parameters,
             QXmlStreamReader &reader);
    void get_feedly(const core::net::Uri::Path &path,
             const core::net::Uri::QueryParameters &parameters,
             QJsonDocument &root);
    void get_readability(const core::net::Uri::Path &path,
             const core::net::Uri::QueryParameters &parameters,
             QJsonDocument &root);
    void get_mercury(const std::string &key, const core::net::Uri::Path &path,
             const core::net::Uri::QueryParameters &parameters,
             QJsonDocument &root);
    QString wget(const std::string uri);
    QString wget(const std::string uri, QXmlStreamReader &reader);
    /**
     * Progress callback that allows the query to cancel pending HTTP requests.
     */
    core::net::http::Request::Progress::Next progress_report(
            const core::net::http::Request::Progress& progress);

    /**
     * Hang onto the configuration information
     */
    config::Base::Ptr config_;

    /**
     * Thread-safe cancelled flag
     */
    std::atomic<bool> cancelled_;
};

/**************************************************************************/
class RedirectException: public std::exception
{
public:
  virtual const char* what() const throw()
  {
    return "HTTP Redirect";
  }
  RedirectException(std::string newurl): std::exception(){
      _newurl = newurl;
  }
  std::string _newurl;
};

}

#endif // CLIENTWEB_H_

