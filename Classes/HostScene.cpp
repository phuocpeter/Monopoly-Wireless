#include "HostScene.h"
#include "ClientScene.h"
#include "PlayerScene.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"
#include "extensions/cocos-ext.h"
#include "network/HttpClient.h"
#include "editor-support/spine/Json.h"
#include <vector>

USING_NS_CC;
USING_NS_CC_EXT;

Scene* HostScene::createScene()
{
  // 'scene' is an autorelease object
  auto scene = Scene::create();
  
  // 'layer' is an autorelease object
  auto layer = HostScene::create();
  
  // add layer as a child to scene
  scene->addChild(layer);
  
  // return the scene
  return scene;
}

// on "init" you need to initialize your instance
bool HostScene::init()
{
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() )
  {
    return false;
  }
  
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  
  // Exit button
  auto exitBtn = ui::Button::create("res/x-64.png");
  exitBtn->setPosition(Vec2(origin.x + exitBtn->getContentSize().width / 4,
                            origin.y + visibleSize.height - exitBtn->getContentSize().height / 4));
  exitBtn->setScale(0.5);
  // Lambda function for exiting
  exitBtn->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type){
    switch (type)
    {
      case ui::Widget::TouchEventType::ENDED:
        Director::getInstance()->end();
        #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
          exit(0);
        #endif
        break;
      default:
        break;
    }
  });
  this->addChild(exitBtn);
  
  // Server Address textfield
  auto *serverField = ui::TextField::create("Địa chỉ Server", "fonts/arial.ttf", 20.0f);
  serverField->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                origin.y + visibleSize.height * 3 / 4));
  serverField->addEventListener(CC_CALLBACK_2(HostScene::serverAddressFieldEvent, this));
  this->addChild(serverField);
  
  // Start money textfield
  auto *startMoneyField = ui::TextField::create("Tiền ban đầu", "fonts/arial.ttf", 20.0f);
  startMoneyField->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                    serverField->getPosition().y - 40));
  startMoneyField->addEventListener(CC_CALLBACK_2(HostScene::startMoneyFieldEvent, this));
  this->addChild(startMoneyField);
  
  // Player name textfield
  auto *playerNameField = ui::TextField::create("Tên người chơi", "fonts/arial.ttf", 20.0f);
  playerNameField->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                    startMoneyField->getPosition().y - 40));
  playerNameField->addEventListener(CC_CALLBACK_2(HostScene::playerNameFieldEvent, this));
  this->addChild(playerNameField);
  
  // Error label
  _errorLabel = Label::createWithTTF("", "fonts/arial.ttf", 20.0f);
  _errorLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                               origin.y + visibleSize.height * 3 / 4 + 40));
  _errorLabel->setTextColor(Color4B::RED);
  _errorLabel->setAlignment(TextHAlignment::CENTER);
  _errorLabel->setVisible(false);
  this->addChild(_errorLabel);
  
  // Create session Button
  _createSessionBtn = ui::Button::create();
  _createSessionBtn->setTitleText("Tạo Game");
  _createSessionBtn->setTitleFontName("fonts/arial.ttf");
  _createSessionBtn->setTitleFontSize(20.0f);
  _createSessionBtn->setPosition(Vec2(origin.x + visibleSize.width / 2,
                           playerNameField->getPosition().y - 40));
  _createSessionBtn->addTouchEventListener(CC_CALLBACK_2(HostScene::createSessionEvent, this));
  this->addChild(_createSessionBtn);
  
  // Loading Bar placeholder
  auto loadingPlaceholder = Sprite::create("res/Slider_Back.png");
  loadingPlaceholder->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                       _createSessionBtn->getPositionY() - 40));
  this->addChild(loadingPlaceholder);
  
  // Loading Bar
  _loadingBar = ui::LoadingBar::create("res/Slider_PressBar.png");
  _loadingBar->setPercent(0);
  _loadingBar->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                _createSessionBtn->getPositionY() - 40));
  this->addChild(_loadingBar);
  
  // Create HTTP Request
  _request = new network::HttpRequest();
  _request->setRequestType(network::HttpRequest::Type::POST);
  _request->setTag("POST createSession");
  _request->setResponseCallback(CC_CALLBACK_2(HostScene::onHttpRequestCompleted, this));
  
  // Create menu tab
  auto hostLabel = Label::createWithTTF("Tạo Game", "fonts/arial.ttf", 20.0f);
  auto host = MenuItemLabel::create(hostLabel);
  host->setEnabled(true);
  
  auto clientLabel = Label::createWithTTF("Vào Game", "fonts/arial.ttf", 20.0f);
  auto client = MenuItemLabel::create(clientLabel);
  client->setEnabled(true);
  client->setCallback(CC_CALLBACK_1(HostScene::clientSceneChange, this));
  
  auto menu = Menu::create(host, client, NULL);
  menu->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 20));
  menu->alignItemsHorizontallyWithPadding(visibleSize.width / 5);
  this->addChild(menu);
  
  return true;
}

// MARK: Handle text input

void HostScene::serverAddressFieldEvent(Ref* sender, ui::TextField::EventType type) {
  ui::TextField* field = dynamic_cast<ui::TextField*>(sender);
  switch (type) {
    case ui::TextField::EventType::INSERT_TEXT:
    case ui::TextField::EventType::DELETE_BACKWARD:
      _serverAddress = field->getString().c_str();
      break;
    case ui::TextField::EventType::ATTACH_WITH_IME:
      _createSessionBtn->setEnabled(false);
      break;
    case ui::TextField::EventType::DETACH_WITH_IME:
      _createSessionBtn->setEnabled(true);
      break;
    default:
      break;
  }
}

void HostScene::startMoneyFieldEvent(Ref* sender, ui::TextField::EventType type) {
  ui::TextField* field = dynamic_cast<ui::TextField*>(sender);
  switch (type) {
    case ui::TextField::EventType::INSERT_TEXT:
    case ui::TextField::EventType::DELETE_BACKWARD:
      _startMoney = atoi(field->getString().c_str());
      break;
    case ui::TextField::EventType::ATTACH_WITH_IME:
      _createSessionBtn->setEnabled(false);
      break;
    case ui::TextField::EventType::DETACH_WITH_IME:
      _createSessionBtn->setEnabled(true);
      break;
    default:
      break;
  }
}

void HostScene::playerNameFieldEvent(Ref* sender, ui::TextField::EventType type) {
  ui::TextField* field = dynamic_cast<ui::TextField*>(sender);
  switch (type) {
    case ui::TextField::EventType::INSERT_TEXT:
    case ui::TextField::EventType::DELETE_BACKWARD:
      _playerName = field->getString().c_str();
      break;
    case ui::TextField::EventType::ATTACH_WITH_IME:
      _createSessionBtn->setEnabled(false);
      break;
    case ui::TextField::EventType::DETACH_WITH_IME:
      _createSessionBtn->setEnabled(true);
      break;
    default:
      break;
  }
}

// MARK: Handle button event

void HostScene::createSessionEvent(Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
  switch (type) {
    case ui::Widget::TouchEventType::BEGAN:
      if (strlen(std::to_string(_startMoney).c_str()) >= 1 && strlen(_playerName.c_str()) >= 1 && strlen(_serverAddress.c_str()) >= 1) {
        _loadingBar->setPercent(5);
      // Concatenate the plain data string
      _plainSendData = "playerName=" + _playerName + "&startMoney=" + std::to_string(_startMoney);
      // Turn plain data string to const char
      _dataToSend = _plainSendData.c_str();
        _loadingBar->setPercent(10);
      // Set _request's parameters
      _request->setUrl(("http://" + _serverAddress + "/monopoly/").c_str());
      _request->setRequestData(_dataToSend, strlen(_dataToSend));
      network::HttpClient::getInstance()->send(_request);
        _loadingBar->setPercent(20);
      } else {
        _loadingBar->setPercent(0);
        // Display error when missing info
        _errorLabel->setString("Vui lòng nhập đủ thông tin");
        _errorLabel->setVisible(true);
      }
      break;
    default:
      break;
  }
}

// MARK: Menu Button Callback

void HostScene::clientSceneChange(cocos2d::Ref *sender) {
  auto director = Director::getInstance();
  auto clientScene = ClientScene::createScene();
  director->replaceScene(clientScene);
}


// MARK: Network Handler

void HostScene::onHttpRequestCompleted(network::HttpClient *sender, network::HttpResponse *response) {
  printf("\n");
  // Print connection status
  if (response->getResponseCode() == 200) {
    printf("Connected successfully\n");
    printf("Creating new session\n");
    
    _loadingBar->setPercent(40);
    std::vector<char> *buffer = response->getResponseData();
    
    // Convert the response data to plain string
    char *concatenated = (char*)malloc(buffer->size());
    std::string string2(buffer->begin(), buffer->end());
    strcpy(concatenated, string2.c_str());
    _loadingBar->setPercent(50);
    
    Json* json = Json_create(concatenated);
    auto errorCode = Json_getInt(json, "errorCode", -99);
    _loadingBar->setPercent(75);
    
    if (errorCode == -1) {
      // No error, switch to next scene
      _loadingBar->setPercent(100);
      auto scene = PlayerScene::createScene(concatenated, _serverAddress, "");
      auto director = Director::getInstance();
      director->replaceScene(scene);
    } else {
      // Error from server
      _loadingBar->setPercent(0);
      auto message = Json_getString(json, "message", "NULL");
      _errorLabel->setString(message);
      _errorLabel->setVisible(true);
      printf("%s\n", message);
    }
  } else {
    // Failed to connect to server
    _loadingBar->setPercent(0);
    _errorLabel->setString("Không thể kết nối tới Server\nLỗi: " + std::to_string(response->getResponseCode()));
    _errorLabel->setVisible(true);
    printf("Error: %li", response->getResponseCode());
  }

}

