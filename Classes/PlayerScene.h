//
//  PlayerScene.hpp
//  MGM
//
//  Created by Tran Thai Phuoc on 2016-06-13.
//
//

#ifndef __PLAYER_SCENE_H__
#define __PLAYER_SCENE_H__

#include "cocos2d.h"
#include "extensions/cocos-ext.h"
#include "ui/CocosGUI.h"
#include "network/HttpClient.h"

USING_NS_CC;
USING_NS_CC_EXT;

class PlayerScene: public Layer
{
public:
  static cocos2d::Scene* createScene(char* jsonString, std::string serverAddress, std::string info);
  
  virtual bool init();
  
  // implement the "static create()" method manually
  CREATE_FUNC(PlayerScene);
  
  // Textfield Handler
  void amountTextFieldEvent(Ref* sender, ui::TextField::EventType type);
  
  // Button handler
  void sendButtonTapped(Ref* sender, ui::Widget::TouchEventType type);
  void refreshButtonTapped(Ref* sender, ui::Widget::TouchEventType type);
  void bankSceneChange(Ref* sender);
  
  // Slider handler
  void sliderEvent(Ref* sender, ui::Slider::EventType type);
  
  // Network Handler
  void onHttpRequestCompleted(network::HttpClient* sender, network::HttpResponse* response);
  
private:
  Sprite* _loadingPlaceholder;
  network::HttpRequest *_request;
  Label* _recipentLabel;
  Label* _errorLabel;
  ui::Button* _sendBtn;
  ui::Button* _refreshBtn;
  ui::LoadingBar* _loadingBar;
  
  std::string _plainSendData;
  std::string _sessionID;
  std::string _playerName;
  std::string _recipentName;
  const char* _dataToSend;
  const char* _player1;
  const char* _player2;
  const char* _player3;
  int _numOfPlayers;
  int _amount;
};

#endif /* __PLAYER_SCENE_H__ */
