//
//  BankScene.hpp
//  MGM
//
//  Created by Tran Thai Phuoc on 2016-06-15.
//
//

#ifndef __BANK_SCENE_H__
#define __BANK_SCENE_H__

#include "cocos2d.h"
#include "extensions/cocos-ext.h"
#include "ui/CocosGUI.h"
#include "network/HttpClient.h"

USING_NS_CC;
USING_NS_CC_EXT;

class BankScene: public Layer
{
public:
  static cocos2d::Scene* createScene(char* jsonString, std::string serverIP, std::string message);
  
  virtual bool init();
  
  // implement the "static create()" method manually
  CREATE_FUNC(BankScene);
  
  // Handle text input
  void amountInputEvent(Ref* sender, ui::TextField::EventType type);
  
  // Handle button
  void playerSceneChange(Ref* sender);
  void sendButtonTapped(Ref* sender, ui::Widget::TouchEventType type);
  
  // Handle slider
  void sliderEvent(Ref* sender, ui::Slider::EventType type);
  
  // Handle network connection
  void onHttpRequestCompleted(network::HttpClient* sender, network::HttpResponse* response);
  
private:
  network::HttpRequest *_request;
  Label* _recipentLabel;
  Label* _errorLabel;
  ui::Button* _sendBtn;
  ui::LoadingBar* _loadingBar;
  
  std::string _plainSendData;
  std::string _sessionID;
  std::string _recipentName;
  const char* _dataToSend;
  const char* _player1;
  const char* _player2;
  const char* _player3;
  const char* _player4;
  int _numOfPlayers;
  int _amount;
};

#endif /* __BANK_SCENE_H__ */
