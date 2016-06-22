//
//  ClientScene.hpp
//  MGM
//
//  Created by Tran Thai Phuoc on 2016-06-10.
//
//

#ifndef __CLIENT_SCENE_H__
#define __CLIENT_SCENE_H__

#include "cocos2d.h"
#include "extensions/cocos-ext.h"
#include "ui/CocosGUI.h"
#include "network/HttpClient.h"

USING_NS_CC;
USING_NS_CC_EXT;

class ClientScene: public Layer
{
public:
  static cocos2d::Scene* createScene();
  
  virtual bool init();
  
  // implement the "static create()" method manually
  CREATE_FUNC(ClientScene);
  
  // Handle text input
  void serverAddressFieldEvent(Ref* sender, ui::TextField::EventType type);
  void sessionIDFieldEvent(Ref* sender, ui::TextField::EventType type);
  void playerNameFieldEvent(Ref* sender, ui::TextField::EventType type);
  
  // Handle button click
  void joinSessionEvent(Ref* sender, ui::Widget::TouchEventType type);
  
  void hostSceneChange(Ref* sender);
  
  void onHttpRequestCompleted(network::HttpClient *sender, network::HttpResponse *response);
  
private:
  ui::Button* _joinSessionBtn;
  ui::LoadingBar* _loadingBar;
  Label* _errorLabel;
  network::HttpRequest* _request;
  const char* _dataToSend;
  std::string _sessionID;
  std::string _playerName;
  std::string _serverAddress;
  std::string _plainSendData;
};

#endif /* __CLIENT_SCENE_H__ */
