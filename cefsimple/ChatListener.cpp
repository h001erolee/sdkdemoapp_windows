#include "ChatListener.h"
#include "application.h"
#include "emattributevalue.h"
#include "json/json.h"

string GetPathForWebPage(const string& localPath)
{
	CefStringUTF8 sURL(localPath);
	CString strPath(sURL.ToWString().c_str());
	strPath.Replace(L"\\", L"/");
	CefString sPath(strPath);
	string url = "file:///";
	url += sPath.ToString();
	return url;
}

void ChatListener::onReceiveMessages(const EMMessageList &messages) 
{
	HANDLE hObject[2];
	hObject[0] = Utils::g_RosterDownloaded;
	hObject[1] = Utils::g_GroupListDownloaded;
	WaitForMultipleObjects(2, hObject, TRUE, INFINITE);


	for (EMMessagePtr msg : messages)
	{
		EMMessage::EMChatType type = msg->chatType();
		string sChatType = "chat";
		if (type == EMMessage::GROUP)
		{
			sChatType = "groupchat";
		}
		else if (type == EMMessage::CHATROOM)
		{
			sChatType = "chatroom";
		}
		const vector<EMMessageBodyPtr> &bodies = msg->bodies();
		const EMMessageBodyPtr _body = bodies[0];
		switch (_body->type())
		{
		case EMMessageBody::TEXT:
		{
			onTextMessage(msg, _body, sChatType);
			break;
		}
		case EMMessageBody::FILE:
		{
			onFileMessage(msg, _body, sChatType);
			break;
		}
		case EMMessageBody::IMAGE:
		{
			onImageMessage(msg, _body, sChatType);
			break;
		}
		case EMMessageBody::VOICE:
		{
			onVoiceMessage(msg, _body, sChatType);
			break;
		}
		case EMMessageBody::COMMAND:
			break;
		case EMMessageBody::VIDEO:
		{
			onVideoMessage(msg, _body, sChatType);
			break;
		}
		case EMMessageBody::LOCATION:
		{
			onLocationMessage(msg, _body, sChatType);
			break;
		}
		}
	}
}

void ChatListener::onTextMessage(const EMMessagePtr msg, const EMMessageBodyPtr _body, string sChatType)
{
	string sRoomType = "undefined";
	if (sChatType.compare("groupchat") == 0)
	{
		sRoomType = "groups";
	}
	else if (sChatType.compare("chatroom") == 0)
	{
		sChatType = "groupchat";
		sRoomType = "chatrooms";
	}

	EMTextMessageBodyPtr body = std::dynamic_pointer_cast<EMTextMessageBody, EMMessageBody>(_body);
	std::stringstream stream;
	stream << "Demo.conn.onTextMessage('{id: \"";
	stream << msg->msgId();
	stream << "\",type : \"";
	stream << sChatType;
	stream << "\", roomtype : \"";
	stream << sRoomType;
	stream << "\", from : \"";
	stream << msg->from();
	stream << "\",to : \"";
	stream << msg->to();
	stream << "\",data : \"";
	stream << Utils::URLEncode(body->text());
    stream << "\"";
    std::map<std::string, std::shared_ptr<easemob::EMAttributeValue> > ext = msg->ext();
    if (ext.size() > 0)
    {
        stream << ",ext : {";
        //string extText;
        //Json::Value root;
        std::map<std::string, std::shared_ptr<EMAttributeValue> >::iterator it;
        int len = ext.size();
        int j = 0;
        for (it = ext.begin(); it != ext.end(); it++)
        {
            stream << it->first;
            stream << ":";
            if (it->second->is<bool>())
            {
                stream << std::boolalpha << it->second->value<bool>();
            }
            else if (it->second->is<char>())
            {
                stream << it->second->value<char>();
            }
            else if (it->second->is<unsigned char>())
            {
                stream << it->second->value<unsigned char>();
            }
            else if (it->second->is<short>())
            {
                stream << it->second->value<short>();
            }
            else if (it->second->is<unsigned short>())
            {
                stream << it->second->value<unsigned short>();
            }
            else if (it->second->is<int32_t>())
            {
                stream << it->second->value<int32_t>();
            }
            else if (it->second->is<uint32_t>())
            {
                stream << it->second->value<uint32_t>();
            }
            else if (it->second->is<int64_t>())
            {
                stream << (uint32_t)it->second->value<int64_t>();
            }
            else if (it->second->is<float>())
            {
                stream << it->second->value<float>();
            }
            else if (it->second->is<double>())
            {
                stream << it->second->value<double>();
            }
            else if (it->second->is<std::string>())
            {
                stream << "\"";
                stream << it->second->value<std::string>();
                stream << "\"";
            }
            else if (it->second->is<std::vector<std::string>>())
            {
                auto sec = it->second->value<std::vector<std::string>>();
                stream << "[";
                for (auto i = sec.begin(); i != sec.end(); i++)
                {
                    stream << "\"";
                    stream << *i;
                    stream << "\"";
                }
                stream << "]";
            }
            else if (it->second->is<EMJsonString>())
            {
                stream << it->second->value<EMJsonString>();
            }
            if (++j < len)
            {
                stream << ",";
            }
        }
        stream << "}";
    }
    stream << "}');";
    Utils::CallJS(stream);
}

string ChatListener::getJSHead(const EMMessagePtr msg, string sChatType, string JSFuncName)
{
	string sRoomType = "undefined";
	if (sChatType.compare("groupchat") == 0)
	{
		sRoomType = "groups";
	}
	else if (sChatType.compare("chatroom") == 0)
	{
		sChatType = "groupchat";
		sRoomType = "chatrooms";
	}

	std::stringstream streamHead;
	streamHead << "Demo.conn.";
	streamHead << JSFuncName;
	streamHead << "('{ext : \"\",id: \"";
	streamHead << msg->msgId();
	streamHead << "\",type : \"";
	streamHead << sChatType;
	streamHead << "\", roomtype : \"";
	streamHead << sRoomType;
	streamHead << "\", from : \"";
	streamHead << msg->from();
	streamHead << "\",to : \"";
	streamHead << msg->to();
    streamHead << "\"";
    std::map<std::string, std::shared_ptr<easemob::EMAttributeValue> > ext = msg->ext();
    if (ext.size() > 0)
    {
        streamHead << ",ext : ";
        string extText;
        std::map<std::string, std::shared_ptr<EMAttributeValue> >::iterator it;
        for (it = ext.begin(); it != ext.end(); it++)
        {
            extText += "\"";
            extText += it->first;
            extText += "\":\"";
            extText += it->second->value<string>();
            extText += "\",";
        }
        if (!extText.empty())
        {
            string tmp = extText.substr(0, extText.length() - 1);
            extText = "{" + tmp + "}";
        }
        streamHead << extText;
    }

	streamHead << ",url : \"";
	string sRet = streamHead.str();
	streamHead.clear();
	streamHead.str("");
	return sRet;
}

string ChatListener::getJSTail(const EMMessageBodyPtr _body, string type)
{
	EMFileMessageBodyPtr body = std::dynamic_pointer_cast<EMFileMessageBody, EMMessageBody>(_body);
	std::stringstream streamTail;
	streamTail << "\",filename : \"";
	streamTail << body->displayName();
	streamTail << "\",file_length : \"";
	streamTail << body->fileLength();
	streamTail << "\",filetype : \"";
	streamTail << type;
	streamTail << "\"}');";
	string sRet = streamTail.str();
	streamTail.clear();
	streamTail.str("");
	return sRet;
}

void ChatListener::CallJSWithoutFilePath(string strJSHead, string strJSTail)
{
	std::stringstream stream;
	stream << strJSHead;
	stream << strJSTail;
	Utils::CallJS(stream);
	stream.clear();
	stream.str("");
}

void ChatListener::CallJSWithFilePath(string strJSHead, string strJSTail, string strPath)
{
	std::stringstream streamAll;
	streamAll << strJSHead;
	streamAll << strPath;
	streamAll << strJSTail;
	Utils::CallJS(streamAll);
	streamAll.clear();
	streamAll.str("");
}

void ChatListener::onFileMessage(const EMMessagePtr msg, const EMMessageBodyPtr _body, string sChatType)
{
	EMFileMessageBodyPtr body = std::dynamic_pointer_cast<EMFileMessageBody, EMMessageBody>(_body);

	string strJSHead = getJSHead(msg, sChatType,"onFileMessage");
	string strJSTail = getJSTail(_body, "file");

	CallJSWithoutFilePath(strJSHead, strJSTail);

	EMCallbackPtr msgCallback(new EMCallback(m_coh,
		[=](void)->bool
	{
		if (EMFileMessageBody::SUCCESSED == body->downloadStatus())
		{
			CallJSWithFilePath(strJSHead, strJSTail, GetPathForWebPage(body->localPath()));
		}
		return true;
	},
		[=](const easemob::EMErrorPtr)->bool
	{
		return false;
	},
		[](int){}));
	msg->setCallback(msgCallback);

	g_client->getChatManager().downloadMessageAttachments(msg);
}

void ChatListener::onImageMessage(const EMMessagePtr msg, const EMMessageBodyPtr _body, string sChatType)
{
	EMImageMessageBodyPtr body = std::dynamic_pointer_cast<EMImageMessageBody, EMMessageBody>(_body);

	string strJSHead = getJSHead(msg, sChatType,"onPictureMessage");
	string strJSTail = getJSTail(_body, "img");

	CallJSWithoutFilePath(strJSHead, strJSTail);

	EMCallbackPtr msgCallback(new EMCallback(m_coh,
		[=](void)->bool
	{
		if (EMFileMessageBody::SUCCESSED == body->downloadStatus() && EMFileMessageBody::SUCCESSED == body->thumbnailDownloadStatus())
		{
			CallJSWithFilePath(strJSHead, strJSTail, GetPathForWebPage(body->localPath()));
		}
		return true;
	},
		[=](const easemob::EMErrorPtr)->bool
	{
		return false;
	},
		[](int){}));
	msg->setCallback(msgCallback);

	g_client->getChatManager().downloadMessageAttachments(msg);
}

void ChatListener::onVoiceMessage(const EMMessagePtr msg, const EMMessageBodyPtr _body, string sChatType)
{
	EMVoiceMessageBodyPtr body = std::dynamic_pointer_cast<EMVoiceMessageBody, EMMessageBody>(_body);

	string strJSHead = getJSHead(msg, sChatType,"onAudioMessage");
	string strJSTail = getJSTail(_body, "audio");

	CallJSWithoutFilePath(strJSHead, strJSTail);

	EMCallbackPtr msgCallback(new EMCallback(m_coh,
		[=](void)->bool
	{
		if (EMFileMessageBody::SUCCESSED == body->downloadStatus())
		{
			CallJSWithFilePath(strJSHead, strJSTail, GetPathForWebPage(body->localPath()));
		}
		return true;
	},
		[=](const easemob::EMErrorPtr)->bool
	{
		return true;
	},
		[](int){}));
	if (EMFileMessageBody::SUCCESSED == body->downloadStatus())
	{
		CallJSWithFilePath(strJSHead, strJSTail, GetPathForWebPage(body->localPath()));
	}
	else
	{
		msg->setCallback(msgCallback);
	}
}

void ChatListener::onVideoMessage(const EMMessagePtr msg, const EMMessageBodyPtr _body, string sChatType)
{
	EMVideoMessageBodyPtr body = std::dynamic_pointer_cast<EMVideoMessageBody, EMMessageBody>(_body);

	string strJSHead = getJSHead(msg, sChatType,"onVideoMessage");
	string strJSTail = getJSTail(_body, "video");

	CallJSWithoutFilePath(strJSHead, strJSTail);

	EMCallbackPtr msgCallback(new EMCallback(m_coh,
		[=](void)->bool
	{
		if (EMFileMessageBody::SUCCESSED == body->downloadStatus() && EMFileMessageBody::SUCCESSED == body->thumbnailDownloadStatus())
		{
			CallJSWithFilePath(strJSHead, strJSTail, GetPathForWebPage(body->localPath()));
		}
		return true;
	},
		[=](const easemob::EMErrorPtr)->bool
	{
		return false;
	},
		[](int){}));
	msg->setCallback(msgCallback);

	g_client->getChatManager().downloadMessageAttachments(msg);
}

void ChatListener::onLocationMessage(const EMMessagePtr msg, const EMMessageBodyPtr _body, string sChatType)
{
	EMLocationMessageBodyPtr body = std::dynamic_pointer_cast<EMLocationMessageBody, EMMessageBody>(_body);
	std::stringstream stream;
	stream << "Demo.conn.onLocationMessage('{id: \"";
	stream << msg->msgId();
	stream << "\",type : \"";
	stream << sChatType;
	stream << "\", from : \"";
	stream << msg->from();
	stream << "\",to : \"";
	stream << msg->to();
	stream << "\",addr : \"";
	stream << body->address();
	stream << "\",lat : \"";
	stream << body->latitude();
	stream << "\",lng : \"";
	stream << body->longitude();
	stream << "\",ext : \"\"}');";
	Utils::CallJS(stream);
}
