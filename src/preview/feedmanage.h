#ifndef PREVIEW_FEEDMANAGE_H
#define PREVIEW_FEEDMANAGE_H

#include <unity/scopes/PreviewQueryBase.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/Variant.h>
#include <types.h>

namespace unity {
namespace scopes {
class Result;
}
}

namespace preview {

    class FeedManage : public unity::scopes::PreviewQueryBase
    {
    public:
        FeedManage(const unity::scopes::Result &result,
             const unity::scopes::ActionMetadata &metadata, config::Base::Ptr action_config);

        ~FeedManage() = default;

        void cancelled() override;

        /**
         * Populates the reply object with preview information.
         */
        void run(unity::scopes::PreviewReplyProxy const& reply) override;
     private:
        unity::scopes::PreviewWidget generator_actions(const std::string &name, std::string label);
        unity::scopes::Variant feed_conf_data_ ;
        feed::Infos _curr_info;
        config::Base::Ptr _config;
    };

}


#endif // PREVIEW_FEED_H
