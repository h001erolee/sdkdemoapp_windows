﻿#include "EasemobCefQueryHandler.h"
#include "commdlg.h"
#include "atlstr.h"
#include <ShlObj.h>
#include "resource.h"

CString GetAppDataPath()
{
	CString appdata_path;
	wchar_t buffer[MAX_PATH] = { 0 };
	BOOL result = SHGetSpecialFolderPath(NULL, buffer, CSIDL_LOCAL_APPDATA, false);
	if (result)
	{
		appdata_path = buffer;
	}
	return appdata_path;
}

string getStringAttrFromJson(Json::Value& json, string attr)
{
	string ret;
	Json::Value defaultValue;
	if (json.get(attr, defaultValue).isString())
	{
		ret = json.get(attr, defaultValue).asString();
	}
	return ret;
}

std::vector<std::string> getArrayAttrFromJson(Json::Value& json, string attr)
{
	std::vector<std::string> ret;
	Json::Value defaultValue;
	if (json.get(attr, defaultValue).isArray())
	{
		Json::Value arr = json.get(attr, defaultValue);
		for (Json::Value member : arr)
		{
			ret.push_back(member.asString());
		}
	}
	return ret;
}

void EasemobCefQueryHandler::InitSDKFunctionMap()
{
	m_mapSDKCall["login"] = &EasemobCefQueryHandler::Login;
	m_mapSDKCall["createAccount"] = &EasemobCefQueryHandler::createAccount;
	m_mapSDKCall["logout"] = &EasemobCefQueryHandler::Logout;
	m_mapSDKCall["getRoster"] = &EasemobCefQueryHandler::getRoster;
	m_mapSDKCall["loadMoreMessages"] = &EasemobCefQueryHandler::loadMoreMessages;
	m_mapSDKCall["getGroup"] = &EasemobCefQueryHandler::getGroup;
	m_mapSDKCall["createGroup"] = &EasemobCefQueryHandler::createGroup;
	m_mapSDKCall["addGroupMembers"] = &EasemobCefQueryHandler::addGroupMembers;
	m_mapSDKCall["removeGroupMembers"] = &EasemobCefQueryHandler::removeGroupMembers;
	m_mapSDKCall["blockGroupMembers"] = &EasemobCefQueryHandler::blockGroupMembers;
	m_mapSDKCall["unblockGroupMembers"] = &EasemobCefQueryHandler::unblockGroupMembers;
	m_mapSDKCall["changeGroupSubject"] = &EasemobCefQueryHandler::changeGroupSubject;
	m_mapSDKCall["changeGroupDescription"] = &EasemobCefQueryHandler::changeGroupDescription;
	m_mapSDKCall["acceptJoinGroupApplication"] = &EasemobCefQueryHandler::acceptJoinGroupApplication;
	m_mapSDKCall["declineJoinGroupApplication"] = &EasemobCefQueryHandler::declineJoinGroupApplication;
	m_mapSDKCall["acceptInvitationFromGroup"] = &EasemobCefQueryHandler::acceptInvitationFromGroup;
	m_mapSDKCall["declineInvitationFromGroup"] = &EasemobCefQueryHandler::declineInvitationFromGroup;
	m_mapSDKCall["getChatroom"] = &EasemobCefQueryHandler::getChatroom;
	m_mapSDKCall["joinChatroom"] = &EasemobCefQueryHandler::joinChatroom;
	m_mapSDKCall["quitChatroom"] = &EasemobCefQueryHandler::quitChatroom;
	m_mapSDKCall["groupMembers"] = &EasemobCefQueryHandler::groupMembers;
	m_mapSDKCall["groupOwner"] = &EasemobCefQueryHandler::groupOwner;
	m_mapSDKCall["groupStyle"] = &EasemobCefQueryHandler::groupStyle;
	m_mapSDKCall["groupSpecification"] = &EasemobCefQueryHandler::groupSpecification;
	m_mapSDKCall["leaveGroup"] = &EasemobCefQueryHandler::leaveGroup;
	m_mapSDKCall["destroyGroup"] = &EasemobCefQueryHandler::destroyGroup;
	m_mapSDKCall["joinPublicGroup"] = &EasemobCefQueryHandler::joinPublicGroup;
	m_mapSDKCall["applyJoinPublicGroup"] = &EasemobCefQueryHandler::applyJoinPublicGroup;
	m_mapSDKCall["addFriend"] = &EasemobCefQueryHandler::addFriend;
	m_mapSDKCall["delFriend"] = &EasemobCefQueryHandler::delFriend;
	m_mapSDKCall["acceptInvitation"] = &EasemobCefQueryHandler::acceptInvitation;
	m_mapSDKCall["declineInvitation"] = &EasemobCefQueryHandler::declineInvitation;
	m_mapSDKCall["getBlacklist"] = &EasemobCefQueryHandler::getBlacklist;
	m_mapSDKCall["addToBlackList"] = &EasemobCefQueryHandler::addToBlackList;
	m_mapSDKCall["removeFromBlackList"] = &EasemobCefQueryHandler::removeFromBlackList;
	m_mapSDKCall["sendMessage"] = &EasemobCefQueryHandler::sendMessage;

	m_mapSDKCallInWorkThread["groupMembers"] = true;
	m_mapSDKCallInWorkThread["createGroup"] = true;
	m_mapSDKCallInWorkThread["addFriend"] = true;
	m_mapSDKCallInWorkThread["delFriend"] = true;
	m_mapSDKCallInWorkThread["getBlacklist"] = true;
	m_mapSDKCallInWorkThread["addToBlackList"] = true;
	m_mapSDKCallInWorkThread["removeFromBlackList"] = true;
	m_mapSDKCallInWorkThread["changeGroupSubject"] = true;
	m_mapSDKCallInWorkThread["changeGroupDescription"] = true;
	m_mapSDKCallInWorkThread["addGroupMembers"] = true;
	m_mapSDKCallInWorkThread["removeGroupMembers"] = true;
	m_mapSDKCallInWorkThread["blockGroupMembers"] = true;
	m_mapSDKCallInWorkThread["unblockGroupMembers"] = true;
	m_mapSDKCallInWorkThread["destroyGroup"] = true;
}

EasemobCefQueryHandler::EasemobCefQueryHandler()
{
	g_client == NULL;
	InitSDKFunctionMap();
	CreateEMClient();
}
#include <thread>
bool EasemobCefQueryHandler::OnQuery(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	int64 query_id,
	const CefString& request,
	bool persistent,
	CefRefPtr<Callback> callback){
	std::string json_document = request.ToString();

	CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();

	Json::Value json;
	Json::Reader reader;
	if (reader.parse(json_document, json))
	{
		string type = getStringAttrFromJson(json, "type");
		if (!type.empty())
		{
			if (type.compare("sendFileMessage") == 0)
			{
				sendFileMessage(json, callback, hwnd);
			}
			else if (m_mapSDKCall[type] != nullptr)
			{
				if (m_mapSDKCallInWorkThread[type])
				{
					std::thread t = thread([=]{
						(this->*m_mapSDKCall[type])(json, callback);
					});
					t.detach();
				}
				else
				{
					(this->*m_mapSDKCall[type])(json, callback);
				}
			}
			return true;
		}
	}
	return false;
}

void EasemobCefQueryHandler::CreateEMClient()
{
	if (g_client == NULL)
	{
		CString strAppDir = GetAppDataPath() + L"\\EasemobDemo";
		CefString sAppDir(strAppDir);
		CreateDirectory(strAppDir, NULL);
		easemob::EMChatConfigsPtr configs(new easemob::EMChatConfigs(sAppDir, sAppDir, "easemob-demo#chatdemoui"));
		configs->setOs(EMChatConfigs::OS_MSWIN);
		configs->setEnableConsoleLog(true);
		configs->setAutoAcceptGroup(false);
		configs->setClientResource("windows");
		configs->setLogLevel(EMChatConfigs::DEBUG_LEVEL);
		EMClient *client = EMClient::create(configs);
		g_client = client;

		mChatListener = new ChatListener();
		g_client->getChatManager().addListener(mChatListener);
		mContactListener = new ContactListener();
		g_client->getContactManager().registerContactListener(mContactListener);
		mConnectionListener = new ConnectionListener();
		g_client->addConnectionListener(mConnectionListener);
		mGroupManagerListener = new GroupManagerListener();
		g_client->getGroupManager().addListener(mGroupManagerListener);
	}
}

EasemobCefQueryHandler::~EasemobCefQueryHandler()
{
}

void EasemobCefQueryHandler::Login(Json::Value json, CefRefPtr<Callback> callback)
{
	CreateEMClient();

	string id = getStringAttrFromJson(json, "id");
	string password = getStringAttrFromJson(json, "password");
	if (!id.empty() && !password.empty())
	{
		EMErrorPtr error = g_client->login(id, password);
		if (error->mErrorCode == EMError::EM_NO_ERROR)
		{
			callback->Success("Login Ok");
			EMError error;
			std::vector<std::string> mContacts;
			mContacts = g_client->getContactManager().allContacts(error);
			if (error.mErrorCode == EMError::EM_NO_ERROR)
			{
				string ret;
				for (string username : mContacts)
				{
					ret += "{\"subscription\":\"both\",\"name\":\"";
					ret += username;
					ret += "\"},";
				}
				if (!ret.empty())
				{
					string tmp = ret.substr(0, ret.length() - 1);
					ret = "Demo.conn._onUpdateMyRoster('[" + tmp + "]')";
				}
				std::stringstream stream;
				stream << ret;
				Utils::CallJS(stream);
				SetEvent(Utils::g_RosterDownloaded);
			}

			EMGroupList groupList = g_client->getGroupManager().allMyGroups(error);
			if (error.mErrorCode == EMError::EM_NO_ERROR)
			{
				string ret;
				for (EMGroupPtr group : groupList)
				{
					ret += "{\"jid\":\"blahblah\",\"name\":\"";
					ret += group->groupSubject();
					ret += "\",\"roomId\":\"";
					ret += group->groupId();
					ret += "\"},";
				}
				if (!ret.empty())
				{
					string tmp = ret.substr(0, ret.length() - 1);
					ret = "Demo.conn._onUpdateMyGroupList('[" + tmp + "]')";
				}
				std::stringstream stream;
				stream << ret;
				Utils::CallJS(stream);
				SetEvent(Utils::g_GroupListDownloaded);
			}
		}
		else
		{
			callback->Failure(error->mErrorCode, error->mDescription);
		}
	}
}

void EasemobCefQueryHandler::createAccount(Json::Value json, CefRefPtr<Callback> callback)
{
	string id = getStringAttrFromJson(json, "id");
	string password = getStringAttrFromJson(json, "password");
	if (!id.empty() && !password.empty())
	{
		EMErrorPtr error = g_client->createAccount(id, password);
		if (error->mErrorCode == EMError::EM_NO_ERROR)
		{
			callback->Success("Sign up Ok");
		}
		else
		{
			callback->Failure(error->mErrorCode, error->mDescription);
		}
	}
}

void EasemobCefQueryHandler::Logout(Json::Value json, CefRefPtr<Callback> callback)
{
	g_client->logout();
	callback->Success("Logout Ok");
	g_client->getChatManager().removeListener(mChatListener);
	g_client->getContactManager().removeContactListener(mContactListener);
	g_client->removeConnectionListener(mConnectionListener);
	g_client->getGroupManager().removeListener(mGroupManagerListener);
	g_client->logout();

	delete mConnectionListener;
	mConnectionListener = NULL;
	delete mContactListener;
	mContactListener = NULL;
	delete mChatListener;
	mChatListener = NULL;
	delete mGroupManagerListener;
	mGroupManagerListener = NULL;
	delete g_client;
	g_client = NULL;
}

void EasemobCefQueryHandler::getRoster(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	std::vector<std::string> mContacts;
	mContacts = g_client->getContactManager().getContactsFromServer(error);
	string ret;
	if (error.mErrorCode == EMError::EM_NO_ERROR)
	{
		for (string username : mContacts)
		{
			ret += "{\"subscription\":\"both\",\"name\":\"";
			ret += username;
			ret += "\"},";
		}
		if (!ret.empty())
		{
			string tmp = ret.substr(0, ret.length() - 1);
			ret = "[" + tmp + "]";
		}
		callback->Success(ret);
		SetEvent(Utils::g_RosterDownloaded);
	}
	else
	{
		callback->Failure(error.mErrorCode, error.mDescription);
	}
}

void EasemobCefQueryHandler::loadMoreMessages(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	string chatType = getStringAttrFromJson(json, "chatType");
	EMConversation::EMConversationType type = EMConversation::CHAT;
	if (chatType == "groupChat")
	{
		type = EMConversation::GROUPCHAT;
	}
	EMConversationPtr con = g_client->getChatManager().conversationWithType(id, type);
	if (con == nullptr)
	{
		return;
	}
	EMMessageList ml = con->loadMoreMessages(-1, 10, EMConversation::DOWN);
	mChatListener->onReceiveMessages(ml);
}

void EasemobCefQueryHandler::getGroup(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	EMGroupList groupList = g_client->getGroupManager().fetchAllMyGroups(error);
	string ret;
	if (error.mErrorCode == EMError::EM_NO_ERROR) {
		for (EMGroupPtr group : groupList)
		{
			ret += "{\"jid\":\"blahblah\",\"name\":\"";
			ret += group->groupSubject();
			ret += "\",\"roomId\":\"";
			ret += group->groupId();
			ret += "\"},";
		}
		if (!ret.empty())
		{
			string tmp = ret.substr(0, ret.length() - 1);
			ret = "[" + tmp + "]";
		}
		string enc = Utils::URLEncode(ret);

		callback->Success(ret);
		SetEvent(Utils::g_GroupListDownloaded);
	}
	else
	{
		callback->Failure(error.mErrorCode, error.mDescription);
	}
}

void EasemobCefQueryHandler::createGroup(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string subject = getStringAttrFromJson(json, "subject");
	string description = getStringAttrFromJson(json, "description");
	string welcomeMessage = getStringAttrFromJson(json, "welcomeMessage");
	string style = getStringAttrFromJson(json, "style");
	string maxUserCount = getStringAttrFromJson(json, "maxUserCount");
	stringstream stream;
	stream << maxUserCount;
	int nMaxUserCount;
	stream >> nMaxUserCount;
	EMMucSetting::EMMucStyle emGroupStyle = EMMucSetting::EMMucStyle::PRIVATE_OWNER_INVITE;
	if (style.compare("PRIVATE_MEMBER_INVITE") == 0)
	{
		emGroupStyle = EMMucSetting::EMMucStyle::PRIVATE_MEMBER_INVITE;
	}
	else if (style.compare("PUBLIC_JOIN_APPROVAL") == 0)
	{
        emGroupStyle = EMMucSetting::EMMucStyle::PUBLIC_JOIN_APPROVAL;
	}
	else if (style.compare("PUBLIC_JOIN_OPEN") == 0)
	{
        emGroupStyle = EMMucSetting::EMMucStyle::PUBLIC_JOIN_OPEN;
	}
	EMMucSetting setting(emGroupStyle,nMaxUserCount, true);
	setting.setStyle(emGroupStyle);
	setting.setMaxUserCount(nMaxUserCount);
	EMMucMemberList members = getArrayAttrFromJson(json, "members");
	EMGroupPtr group = g_client->getGroupManager().createGroup(subject, description, welcomeMessage, setting, members, error);
	string ret;
	if (error.mErrorCode == EMError::EM_NO_ERROR)
	{
		ret = "{\"id\":\"" + group->groupId()
			+ "\",\"subject\":\"" + group->groupSubject() + "\"}";
		callback->Success(ret);
	}
	else
	{
		callback->Failure(error.mErrorCode, error.mDescription);
	}
}

void EasemobCefQueryHandler::addGroupMembers(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	string welcomeMessage = getStringAttrFromJson(json, "welcomeMessage");
	EMMucMemberList members = getArrayAttrFromJson(json, "members");
	EMGroupPtr group = g_client->getGroupManager().addGroupMembers(id, members, welcomeMessage, error);
	string ret;
	if (error.mErrorCode == EMError::EM_NO_ERROR)
	{
		ret = "{\"id\":\"" + group->groupId()
			+ "\",\"subject\":\"" + group->groupSubject() + "\"}";
		callback->Success(ret);
	}
	else
	{
		callback->Failure(error.mErrorCode, error.mDescription);
	}
}

void EasemobCefQueryHandler::removeGroupMembers(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	EMMucMemberList members = getArrayAttrFromJson(json, "members");
	EMGroupPtr group = g_client->getGroupManager().removeGroupMembers(id, members, error);
	string ret;
	if (error.mErrorCode == EMError::EM_NO_ERROR)
	{
		ret = "{\"id\":\"" + group->groupId()
			+ "\",\"subject\":\"" + group->groupSubject() + "\"}";
		callback->Success(ret);
	}
	else
	{
		callback->Failure(error.mErrorCode, error.mDescription);
	}
}

void EasemobCefQueryHandler::blockGroupMembers(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	EMMucMemberList members = getArrayAttrFromJson(json, "members");
	EMGroupPtr group = g_client->getGroupManager().blockGroupMembers(id, members, error);
	string ret;
	if (error.mErrorCode == EMError::EM_NO_ERROR)
	{
		ret = "{\"id\":\"" + group->groupId()
			+ "\",\"subject\":\"" + group->groupSubject() + "\"}";
		callback->Success(ret);
	}
	else
	{
		callback->Failure(error.mErrorCode, error.mDescription);
	}
}

void EasemobCefQueryHandler::unblockGroupMembers(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	EMMucMemberList members = getArrayAttrFromJson(json, "members");
	EMGroupPtr group = g_client->getGroupManager().unblockGroupMembers(id, members, error);
	string ret;
	if (error.mErrorCode == EMError::EM_NO_ERROR)
	{
		ret = "{\"id\":\"" + group->groupId()
			+ "\",\"subject\":\"" + group->groupSubject() + "\"}";
		callback->Success(ret);
	}
	else
	{
		callback->Failure(error.mErrorCode, error.mDescription);
	}
}

void EasemobCefQueryHandler::changeGroupSubject(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	string subject = getStringAttrFromJson(json, "subject");
	EMGroupPtr group = g_client->getGroupManager().changeGroupSubject(id, subject, error);
	string ret;
	if (error.mErrorCode == EMError::EM_NO_ERROR)
	{
		ret = "{\"id\":\"" + group->groupId()
			+ "\",\"subject\":\"" + group->groupSubject() + "\"}";
		callback->Success(ret);
	}
	else
	{
		callback->Failure(error.mErrorCode, error.mDescription);
	}
}

void EasemobCefQueryHandler::changeGroupDescription(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	string description = getStringAttrFromJson(json, "description");
	EMGroupPtr group = g_client->getGroupManager().changeGroupDescription(id, description, error);
	string ret;
	if (error.mErrorCode == EMError::EM_NO_ERROR)
	{
		ret = "{\"id\":\"" + group->groupId()
			+ "\",\"description\":\"" + group->groupDescription() + "\"}";
		callback->Success(ret);
	}
	else
	{
		callback->Failure(error.mErrorCode, error.mDescription);
	}
}

void EasemobCefQueryHandler::acceptJoinGroupApplication(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	string user = getStringAttrFromJson(json, "user");
	EMGroupPtr group = g_client->getGroupManager().acceptJoinGroupApplication(id, user, error);
	string ret;
	if (error.mErrorCode == EMError::EM_NO_ERROR)
	{
		callback->Success(ret);
	}
	else
	{
		callback->Failure(error.mErrorCode, error.mDescription);
	}
}

void EasemobCefQueryHandler::declineJoinGroupApplication(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	string user = getStringAttrFromJson(json, "user");
	string reason = getStringAttrFromJson(json, "reason");
	EMGroupPtr group = g_client->getGroupManager().declineJoinGroupApplication(id, user, reason, error);
	string ret;
	if (error.mErrorCode == EMError::EM_NO_ERROR)
	{
		callback->Success(ret);
	}
	else
	{
		callback->Failure(error.mErrorCode, error.mDescription);
	}
}

void EasemobCefQueryHandler::acceptInvitationFromGroup(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	string user = getStringAttrFromJson(json, "user");
	EMGroupPtr group = g_client->getGroupManager().acceptInvitationFromGroup(id, user, error);
	string ret;
	if (error.mErrorCode == EMError::EM_NO_ERROR)
	{
		callback->Success(ret);
	}
	else
	{
		callback->Failure(error.mErrorCode, error.mDescription);
	}
}

void EasemobCefQueryHandler::declineInvitationFromGroup(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	string user = getStringAttrFromJson(json, "user");
	string reason = getStringAttrFromJson(json, "reason");
	g_client->getGroupManager().declineInvitationFromGroup(id, user, reason, error);
	string ret;
	if (error.mErrorCode == EMError::EM_NO_ERROR)
	{
		callback->Success(ret);
	}
	else
	{
		callback->Failure(error.mErrorCode, error.mDescription);
	}
}

void EasemobCefQueryHandler::getChatroom(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	EMChatroomList roomList = g_client->getChatroomManager().fetchAllChatrooms(error);
	string ret;
	if (error.mErrorCode == EMError::EM_NO_ERROR) {
		for (EMChatroomPtr room : roomList)
		{
			ret += "{\"owner\":\"";
			ret += room->owner();
			ret += "\",\"name\":\"";
			ret += room->chatroomSubject();
			ret += "\",\"id\":\"";
			ret += room->chatroomId();
			ret += "\",\"affiliations_count\":";
			ret += "5";
			ret += "},";
		}
		if (!ret.empty())
		{
			string tmp = ret.substr(0, ret.length() - 1);
			ret = "[" + tmp + "]";
		}
		callback->Success(ret);
	}
	else
	{
		callback->Failure(error.mErrorCode, error.mDescription);
	}
}

void EasemobCefQueryHandler::joinChatroom(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	if (!id.empty())
	{
		g_client->getChatroomManager().joinChatroom(id, error);
		if (error.mErrorCode != EMError::EM_NO_ERROR)
		{
			callback->Failure(error.mErrorCode, error.mDescription);
		}
		else
		{
			callback->Success(id);
		}

	}
}

void EasemobCefQueryHandler::quitChatroom(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	if (!id.empty())
	{
		g_client->getChatroomManager().leaveChatroom(id, error);
		if (error.mErrorCode != EMError::EM_NO_ERROR)
		{
			callback->Failure(error.mErrorCode, error.mDescription);
		}
		else
		{
			callback->Success(id);
		}
	}
}

void EasemobCefQueryHandler::groupMembers(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	if (!id.empty())
	{
		string ret;
		const EMMucMemberList gml = g_client->getGroupManager().fetchGroupSpecification(id, error)->groupMembers();
		for (string member : gml)
		{
			ret += "{\"jid\":\"";
			ret += member;
			ret += "\",\"affiliation\":\"";
			ret += "member";
			ret += "\"},";
		}
		if (!ret.empty())
		{
			string tmp = ret.substr(0, ret.length() - 1);
			ret = "[" + tmp + "]";
		}
		if (error.mErrorCode != EMError::EM_NO_ERROR)
		{
			callback->Failure(error.mErrorCode, error.mDescription);
		}
		else
		{
			callback->Success(ret);
		}
	}
}

void EasemobCefQueryHandler::groupOwner(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	if (!id.empty())
	{
		string ret = g_client->getGroupManager().fetchGroupSpecification(id, error)->groupOwner();
		if (error.mErrorCode != EMError::EM_NO_ERROR)
		{
			callback->Failure(error.mErrorCode, error.mDescription);
		}
		else
		{
			callback->Success(ret);
		}
	}
}

void EasemobCefQueryHandler::groupStyle(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	if (!id.empty())
	{
		const EMMucSetting *setting = g_client->getGroupManager().fetchGroupSpecification(id, error)->groupSetting();
		if (error.mErrorCode != EMError::EM_NO_ERROR)
		{
			callback->Failure(error.mErrorCode, error.mDescription);
		}
		else
		{
			string ret = "PRIVATE_MEMBER_INVITE";
			switch (setting->style())
			{
			case EMMucSetting::PRIVATE_MEMBER_INVITE:
				ret = "PRIVATE_MEMBER_INVITE";
				break;
			case EMMucSetting::PRIVATE_OWNER_INVITE:
				ret = "PRIVATE_OWNER_INVITE";
				break;
			case EMMucSetting::PUBLIC_JOIN_OPEN:
				ret = "PUBLIC_JOIN_OPEN";
				break;
			case EMMucSetting::PUBLIC_JOIN_APPROVAL:
				ret = "PUBLIC_JOIN_APPROVAL";
				break;
			default:
				break;
			}
			callback->Success(ret);
		}
	}
}

void EasemobCefQueryHandler::groupSpecification(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	if (!id.empty())
	{
		EMGroupPtr group = g_client->getGroupManager().fetchGroupSpecification(id, error);
		if (error.mErrorCode != EMError::EM_NO_ERROR)
		{
			callback->Failure(error.mErrorCode, error.mDescription);
		}
		else
		{
			string style = "PRIVATE_MEMBER_INVITE";
			switch (group->groupSetting()->style())
			{
			case EMMucSetting::PRIVATE_MEMBER_INVITE:
				style = "PRIVATE_MEMBER_INVITE";
				break;
			case EMMucSetting::PRIVATE_OWNER_INVITE:
				style = "PRIVATE_OWNER_INVITE";
				break;
			case EMMucSetting::PUBLIC_JOIN_OPEN:
				style = "PUBLIC_JOIN_OPEN";
				break;
			case EMMucSetting::PUBLIC_JOIN_APPROVAL:
				style = "PUBLIC_JOIN_APPROVAL";
				break;
			default:
				break;
			}
			string members;
			const EMMucMemberList gml = group->groupMembers();
			for (string member : gml)
			{
				members += "{\"jid\":\"";
				members += member;
				members += "\",\"affiliation\":\"";
				members += "member";
				members += "\"},";
			}
			if (!members.empty())
			{
				string tmp = members.substr(0, members.length() - 1);
				members = "[" + tmp + "]";
			}
			else
			{
				members = "[]";
			}

			string bans;
			const EMMucMemberList gb = group->groupBans();
			for (string member : gb)
			{
				bans += "{\"jid\":\"";
				bans += member;
				bans += "\",\"affiliation\":\"";
				bans += "member";
				bans += "\"},";
			}
			if (!bans.empty())
			{
				string tmp = bans.substr(0, bans.length() - 1);
				bans = "[" + tmp + "]";
			}
			else
			{
				bans = "[]";
			}

			std::stringstream stream;
			stream << "{\"owner\":\"";
			stream << group->groupOwner();
			stream << "\",\"style\":\"";
			stream << style;
			stream << "\",\"maxUserCount\":\"";
			stream << group->groupSetting()->maxUserCount();
			stream << "\",\"members\":";
			stream << members;
			stream << ",\"bans\":";
			stream << bans;
			stream << "}";

			callback->Success(stream.str());
		}
	}
}

void EasemobCefQueryHandler::leaveGroup(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	if (!id.empty())
	{
		g_client->getGroupManager().leaveGroup(id, error);
		if (error.mErrorCode != EMError::EM_NO_ERROR)
		{
			callback->Failure(error.mErrorCode, error.mDescription);
		}
		else
		{
			callback->Success(id);
		}
	}
}

void EasemobCefQueryHandler::destroyGroup(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	if (!id.empty())
	{
		g_client->getGroupManager().destroyGroup(id, error);
		if (error.mErrorCode != EMError::EM_NO_ERROR)
		{
			callback->Failure(error.mErrorCode, error.mDescription);
		}
		else
		{
			callback->Success(id);
		}
	}
}

void EasemobCefQueryHandler::joinPublicGroup(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	if (!id.empty())
	{
		const EMGroupPtr group = g_client->getGroupManager().joinPublicGroup(id, error);
		if (error.mErrorCode != EMError::EM_NO_ERROR)
		{
			callback->Failure(error.mErrorCode, error.mDescription);
		}
		else
		{
			callback->Success(group->groupId());
		}
	}
}

void EasemobCefQueryHandler::applyJoinPublicGroup(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string id = getStringAttrFromJson(json, "id");
	string nickname = getStringAttrFromJson(json, "nickname");
	string message = getStringAttrFromJson(json, "message");
	if (!id.empty() && !nickname.empty() && !message.empty())
	{
		const EMGroupPtr group = g_client->getGroupManager().applyJoinPublicGroup(id, nickname, message, error);
		if (error.mErrorCode != EMError::EM_NO_ERROR)
		{
			callback->Failure(error.mErrorCode, error.mDescription);
		}
		else
		{
			callback->Success(group->groupId());
		}
	}
}

void EasemobCefQueryHandler::addFriend(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string to = getStringAttrFromJson(json, "to");
	string message = getStringAttrFromJson(json, "message");
	if (!to.empty())
	{
		g_client->getContactManager().inviteContact(to, message, error);
		if (error.mErrorCode != EMError::EM_NO_ERROR)
		{
			callback->Failure(error.mErrorCode, error.mDescription);
		}
		else
		{
			callback->Success(to);
		}
	}
}

void EasemobCefQueryHandler::getBlacklist(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	std::vector<std::string> mContacts;
	mContacts = g_client->getContactManager().getBlackListFromServer(error);
	string ret;
	if (error.mErrorCode == EMError::EM_NO_ERROR)
	{
		for (string username : mContacts)
		{
			ret += "{\"subscription\":\"none\",\"name\":\"";
			ret += username;
			ret += "\"},";
		}
		if (!ret.empty())
		{
			string tmp = ret.substr(0, ret.length() - 1);
			ret = "[" + tmp + "]";
		}
		callback->Success(ret);
	}
	else
	{
		callback->Failure(error.mErrorCode, error.mDescription);
	}
}

void EasemobCefQueryHandler::addToBlackList(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string username = getStringAttrFromJson(json, "username");
	if (!username.empty())
	{
		g_client->getContactManager().addToBlackList(username, true, error);
		if (error.mErrorCode != EMError::EM_NO_ERROR)
		{
			callback->Failure(error.mErrorCode, error.mDescription);
		}
		else
		{
			callback->Success(username);
		}
	}
}

void EasemobCefQueryHandler::removeFromBlackList(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string username = getStringAttrFromJson(json, "username");
	if (!username.empty())
	{
		g_client->getContactManager().removeFromBlackList(username, error);
		if (error.mErrorCode != EMError::EM_NO_ERROR)
		{
			callback->Failure(error.mErrorCode, error.mDescription);
		}
		else
		{
			callback->Success(username);
		}
	}
}

void EasemobCefQueryHandler::delFriend(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string to = getStringAttrFromJson(json, "to");
	if (!to.empty())
	{
		g_client->getContactManager().deleteContact(to, error);
		if (error.mErrorCode != EMError::EM_NO_ERROR)
		{
			callback->Failure(error.mErrorCode, error.mDescription);
		}
		else
		{
			callback->Success(to);
		}
	}
}

void EasemobCefQueryHandler::acceptInvitation(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string to = getStringAttrFromJson(json, "to");
	if (!to.empty())
	{
		g_client->getContactManager().acceptInvitation(to, error);
		if (error.mErrorCode != EMError::EM_NO_ERROR)
		{
			callback->Failure(error.mErrorCode, error.mDescription);
		}
		else
		{
			callback->Success(to);
		}
	}
}

void EasemobCefQueryHandler::declineInvitation(Json::Value json, CefRefPtr<Callback> callback)
{
	EMError error;
	string to = getStringAttrFromJson(json, "to");
	if (!to.empty())
	{
		g_client->getContactManager().declineInvitation(to, error);
		if (error.mErrorCode != EMError::EM_NO_ERROR)
		{
			callback->Failure(error.mErrorCode, error.mDescription);
		}
		else
		{
			callback->Success(to);
		}
	}
}

void EasemobCefQueryHandler::sendMessage(Json::Value json, CefRefPtr<Callback> callback)
{
	string to = getStringAttrFromJson(json, "to");
	string content = getStringAttrFromJson(json, "msg");
	content = Utils::URLDecode(content);
	string chatType = getStringAttrFromJson(json, "chatType");

	EMMessageBodyPtr body = EMTextMessageBodyPtr(new EMTextMessageBody(content.c_str()));

	CefStringUTF8 utf8(content);

	EMMessage::EMChatType type = EMMessage::SINGLE;
	if (chatType.compare("chatRoom") == 0)
	{
		type = EMMessage::CHATROOM;
	}
	else if(chatType.compare("groupChat") == 0)
	{
		type = EMMessage::GROUP;
	}

	EMMessagePtr msg = EMMessage::createSendMessage(g_client->getLoginInfo().loginUser(), to,
		body, type);
	EMCallbackPtr msgCallback(new EMCallback(m_coh,
		[=](void)->bool
	{
		string enc = Utils::URLEncode(utf8);
		callback->Success(enc.c_str());
		return true;
	},
		[=](const easemob::EMErrorPtr error)->bool
	{
		callback->Failure(error->mErrorCode, error->mDescription);
		return false;
	},
		[](int){}));

	msg->setCallback(msgCallback);

	g_client->getChatManager().sendMessage(msg);
}

void EasemobCefQueryHandler::sendFileMessage(Json::Value json, CefRefPtr<Callback> callback, HWND hwnd)
{
	string to = getStringAttrFromJson(json, "to");
	string content = getStringAttrFromJson(json, "msg");
	string sChatType = getStringAttrFromJson(json, "chatType");
	string type = getStringAttrFromJson(json, "message_type");

	OPENFILENAME ofn;       // common dialog box structure
	TCHAR szFile[260];       // buffer for file name

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	const int FILTER_LENGTH = 500;
	TCHAR szFilter[FILTER_LENGTH] = {0};
	LoadString(NULL, IDS_ALL_FILES, szFilter, FILTER_LENGTH);
	ofn.lpstrFilter = szFilter;
	if (type.compare("img") == 0)
	{
		memset(szFilter, 0, FILTER_LENGTH);
		LoadString(NULL, IDS_IMG_FILES, szFilter, FILTER_LENGTH);
		ofn.lpstrFilter = szFilter;
	}
	else if (type.compare("aud") == 0)
	{
		memset(szFilter, 0, FILTER_LENGTH);
		LoadString(NULL, IDS_AUD_FILES, szFilter, FILTER_LENGTH);
		ofn.lpstrFilter = szFilter;
	}
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;


	if (GetOpenFileName(&ofn) == TRUE)
	{
		std::wstring sAttach = ofn.lpstrFile;
		CefStringUTF8 utf8(sAttach);


		EMMessageBodyPtr body;
		if (type.compare("aud") == 0)
		{
			body = EMFileMessageBodyPtr(new EMFileMessageBody(utf8.ToString().c_str()));
		}
		else if (type.compare("img") == 0)
		{
			body = EMImageMessageBodyPtr(new EMImageMessageBody(utf8.ToString().c_str(), ""));
		}
		else if (type.compare("file") == 0)
		{
			body = EMFileMessageBodyPtr(new EMFileMessageBody(utf8.ToString().c_str()));
		}

		EMMessage::EMChatType chatType = EMMessage::SINGLE;
		if (sChatType.compare("chatRoom") == 0)
		{
			chatType = EMMessage::CHATROOM;
		}
		else if (sChatType.compare("groupChat") == 0)
		{
			chatType = EMMessage::GROUP;
		}

		EMMessagePtr msg = EMMessage::createSendMessage(g_client->getLoginInfo().loginUser(), to,
			body, chatType);

		EMCallbackPtr msgCallback(new EMCallback(m_coh,
			[=](void)->bool
		{
			string enc = Utils::URLEncode(utf8);

			string ret = "{\"id\":\"";
			ret += msg->msgId();
			ret += "\",\"url\":\"";
			ret += enc;
			ret += "\"}";
			callback->Success(ret.c_str());
			return true;
		},
			[=](const easemob::EMErrorPtr error)->bool
		{
			callback->Failure(error->mErrorCode, error->mDescription);
			return false;
		},
			[](int){}));

		msg->setCallback(msgCallback);

		g_client->getChatManager().sendMessage(msg);
	}
}
