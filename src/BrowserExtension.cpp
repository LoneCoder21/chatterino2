#include "BrowserExtension.hpp"

#include "singletons/NativeMessaging.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QTimer>

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>

#ifdef Q_OS_WIN
#    include <fcntl.h>
#    include <io.h>
#    include <stdio.h>
#endif

namespace chatterino {

namespace {
    void initFileMode()
    {
#ifdef Q_OS_WIN
        _setmode(_fileno(stdin), _O_BINARY);
        _setmode(_fileno(stdout), _O_BINARY);
#endif
    }

    void runLoop(NativeMessagingClient &client)
    {
        auto received_message = std::make_shared<std::atomic_bool>(true);

        auto thread = std::thread([=]() {
            while (true)
            {
                using namespace std::chrono_literals;
                if (!received_message->exchange(false))
                {
                    _Exit(1);
                }
                std::this_thread::sleep_for(5s);
            }
        });

        while (true)
        {
            char size_c[4];
            std::cin.read(size_c, 4);

            if (std::cin.eof())
            {
                break;
            }

            auto size = *reinterpret_cast<uint32_t *>(size_c);

            std::unique_ptr<char[]> buffer(new char[size + 1]);
            std::cin.read(buffer.get(), size);
            *(buffer.get() + size) = '\0';

            auto data = QByteArray::fromRawData(buffer.get(),
                                                static_cast<int32_t>(size));
            auto doc = QJsonDocument();

            if (doc.object().value("type") == "nm_pong")
            {
                received_message->store(true);
            }

            received_message->store(true);

            client.sendMessage(data);
        }
        _Exit(0);
    }
}  // namespace

void runBrowserExtensionHost()
{
    initFileMode();

    NativeMessagingClient client;

    runLoop(client);
}

}  // namespace chatterino
