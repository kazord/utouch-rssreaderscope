#ifndef PREVIEW_MESSAGE_H
#define PREVIEW_MESSAGE_H

#include <unity/scopes/PreviewQueryBase.h>

namespace unity {
namespace scopes {
class Result;
}
}

namespace preview {

    class Message : public unity::scopes::PreviewQueryBase
    {
    public:
        Message(const unity::scopes::Result &result,
             const unity::scopes::ActionMetadata &metadata);
        Message(const unity::scopes::Result &result,
             const unity::scopes::ActionMetadata &metadata,
             const std::string &message);

        ~Message() = default;

        void cancelled() override;

        /**
         * Populates the reply object with preview information.
         */
        void run(unity::scopes::PreviewReplyProxy const& reply) override;
    private:
        std::string _message;
    };

}


#endif // PREVIEW_MESSAGE_H
