#pragma once

#include <QMainWindow>

#include "../proto/cc_proto_json_serializer.hpp"
#include "../rpc/cc_rpc_base.hpp"
#include "../proto/cc_serializer_base.hpp"
#include <variant>

#include "ilias_qt.hpp"

namespace Ui {
    class MainWindow;
} // namespace UI

class MainWidget : public QMainWindow {
    Q_OBJECT
public:
    MainWidget(ILIAS_NAMESPACE::QIoContext* ctxt, std::shared_ptr<CS_PROTO_NAMESPACE::ProtoFactory> protoFactory, QWidget* parent = nullptr);
    ~MainWidget();

    ILIAS_NAMESPACE::Task<void> clientLoop(std::weak_ptr<CS_RPC_NAMESPACE::ChannelBase> channel);
    ILIAS_NAMESPACE::Task<void> serverLoop();
    ILIAS_NAMESPACE::Task<void> makeMainChannel();
    ILIAS_NAMESPACE::Task<void> sendMessage(std::weak_ptr<CS_RPC_NAMESPACE::ChannelBase> channel);
    ILIAS_NAMESPACE::Task<void> recvMessage(std::weak_ptr<CS_RPC_NAMESPACE::ChannelBase> channel);
    void closeChannel(std::weak_ptr<CS_RPC_NAMESPACE::ChannelBase> channel);

protected Q_SLOTS:
    void startService();
    void onMakeMainChannel();
    void selectedChannel(int index);
    void onSendMessage();
    void closeService();

private:
    Ui::MainWindow *ui;
    ILIAS_NAMESPACE::QIoContext *mCtxt;
    CS_PROTO_NAMESPACE::ChannelFactory mChannelFactor;
    bool mExit = true;
    std::weak_ptr<CS_RPC_NAMESPACE::ChannelBase> mCurrentChannel;
    std::vector<ILIAS_NAMESPACE::CancelHandle> mHandles;
    std::map<int, ILIAS_NAMESPACE::JoinHandle<void>> mClientLoopHandles;
};