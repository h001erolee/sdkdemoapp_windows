#pragma once
#include "include/wrapper/cef_message_router.h"
#include "json/json.h"
#include <emgroupmanager_interface.h>
#include <emchatroommanager_interface.h>
#include <emconversation.h>
#include "application.h"
#include <emclient.h>
#include <emlogininfo.h>
#include "ChatListener.h"
#include "ContactListener.h"
#include "ConnectionListener.h"
#include "GroupManagerListener.h"

using namespace easemob;
using namespace std;

class EasemobCefQueryHandler : public CefMessageRouterBrowserSide::Handler {
public:
	EasemobCefQueryHandler();
	~EasemobCefQueryHandler();

	void InitSDKFunctionMap();
    void CreateEMClient(const string &appDir = "EasemobDemo", const string &appKey = "easemob-demo#chatdemoui", const string &imIP="",
        const string &imPort = "", const string &restIPandPort = "");
	virtual bool OnQuery(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,	int64 query_id,
		const CefString& request,bool persistent,
		CefRefPtr<Callback> callback) override;
	void Login(Json::Value json, CefRefPtr<Callback> callback);
	void createAccount(Json::Value json, CefRefPtr<Callback> callback);
	void Logout(Json::Value json, CefRefPtr<Callback> callback);
	void getRoster(Json::Value json, CefRefPtr<Callback> callback);
	void getGroup(Json::Value json, CefRefPtr<Callback> callback);
	void createGroup(Json::Value json, CefRefPtr<Callback> callback);
	void addGroupMembers(Json::Value json, CefRefPtr<Callback> callback);
    void addGroupAdmin(Json::Value json, CefRefPtr<Callback> callback);
    void transferGroupOwner(Json::Value json, CefRefPtr<Callback> callback);
    void removeGroupAdmin(Json::Value json, CefRefPtr<Callback> callback);
    void changeGroupSubject(Json::Value json, CefRefPtr<Callback> callback);
	void changeGroupDescription(Json::Value json, CefRefPtr<Callback> callback);
	void acceptJoinGroupApplication(Json::Value json, CefRefPtr<Callback> callback);
	void declineJoinGroupApplication(Json::Value json, CefRefPtr<Callback> callback);
	void acceptInvitationFromGroup(Json::Value json, CefRefPtr<Callback> callback);
	void declineInvitationFromGroup(Json::Value json, CefRefPtr<Callback> callback);
	void removeGroupMembers(Json::Value json, CefRefPtr<Callback> callback);
	void blockGroupMembers(Json::Value json, CefRefPtr<Callback> callback);
	void unblockGroupMembers(Json::Value json, CefRefPtr<Callback> callback);
    void muteGroupMembers(Json::Value json, CefRefPtr<Callback> callback);
    void unmuteGroupMembers(Json::Value json, CefRefPtr<Callback> callback);
    void getChatroom(Json::Value json, CefRefPtr<Callback> callback);
	void joinChatroom(Json::Value json, CefRefPtr<Callback> callback);
	void quitChatroom(Json::Value json, CefRefPtr<Callback> callback);
    void groupMembers(Json::Value json, CefRefPtr<Callback> callback);
    void groupMutes(Json::Value json, CefRefPtr<Callback> callback);
    void groupOwner(Json::Value json, CefRefPtr<Callback> callback);
	void groupStyle(Json::Value json, CefRefPtr<Callback> callback);
	void groupSpecification(Json::Value json, CefRefPtr<Callback> callback);
	void leaveGroup(Json::Value json, CefRefPtr<Callback> callback);
	void destroyGroup(Json::Value json, CefRefPtr<Callback> callback);
	void joinPublicGroup(Json::Value json, CefRefPtr<Callback> callback);
	void applyJoinPublicGroup(Json::Value json, CefRefPtr<Callback> callback);
	void addFriend(Json::Value json, CefRefPtr<Callback> callback);
	void getBlacklist(Json::Value json, CefRefPtr<Callback> callback);
	void addToBlackList(Json::Value json, CefRefPtr<Callback> callback);
	void removeFromBlackList(Json::Value json, CefRefPtr<Callback> callback);
	void delFriend(Json::Value json, CefRefPtr<Callback> callback);
	void acceptInvitation(Json::Value json, CefRefPtr<Callback> callback);
	void declineInvitation(Json::Value json, CefRefPtr<Callback> callback);
	void sendMessage(Json::Value json, CefRefPtr<Callback> callback);
	void sendFileMessage(Json::Value json, CefRefPtr<Callback> callback, HWND hwnd);
	void loadMoreMessages(Json::Value json, CefRefPtr<Callback> callback);

private:
	EMCallbackObserverHandle m_coh;
	typedef void (EasemobCefQueryHandler::*fnSDKCall)(Json::Value, CefRefPtr<Callback>);
	map<string, fnSDKCall> m_mapSDKCall;
	map<string, bool> m_mapSDKCallInWorkThread;
	ChatListener *mChatListener;
	ContactListener * mContactListener;
	ConnectionListener *mConnectionListener;
	GroupManagerListener *mGroupManagerListener;
    string m_workDir;
    string m_appKey;
};