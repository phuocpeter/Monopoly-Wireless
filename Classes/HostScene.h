#ifndef __HOST_SCENE_H__
#define __HOST_SCENE_H__

#include "cocos2d.h"
#include "extensions/cocos-ext.h"
#include "ui/CocosGUI.h"
#include "network/HttpClient.h"

USING_NS_CC;
USING_NS_CC_EXT;

class HostScene : public cocos2d::Layer
{
public:
  static cocos2d::Scene* createScene();
  
  virtual bool init();
  
  // implement the "static create()" method manually
  CREATE_FUNC(HostScene);
  
  // Handle text input
  void serverAddressFieldEvent(Ref* sender, ui::TextField::EventType type);
  void startMoneyFieldEvent(Ref* sender, ui::TextField::EventType type);
  void playerNameFieldEvent(Ref* sender, ui::TextField::EventType type);
  
  // Handle button click
  void createSessionEvent(Ref* sender, ui::Widget::TouchEventType type);
  
  void clientSceneChange(Ref* sender);
  
  void onHttpRequestCompleted(network::HttpClient *sender, network::HttpResponse *response);
  
private:
  ui::Button* _createSessionBtn;
  ui::LoadingBar* _loadingBar;
  Label* _errorLabel;
  network::HttpRequest* _request;
  
  const char* _dataToSend;
  int _startMoney;
  std::string _playerName;
  std::string _serverAddress;
  std::string _plainSendData;
};

#endif // __HELLOWORLD_SCENE_H__
