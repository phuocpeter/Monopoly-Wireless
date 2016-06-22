//
//  BankScene.cpp
//  MGM
//
//  Created by Tran Thai Phuoc on 2016-06-15.
//
//

#include "BankScene.h"
#include "PlayerScene.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"
#include "extensions/cocos-ext.h"
#include "network/HttpClient.h"
#include "editor-support/spine/Json.h"
#include <vector>

USING_NS_CC;
USING_NS_CC_EXT;

char* js;
std::string serverAddress;
std::string info;

Scene* BankScene::createScene(char* jsonString, std::string serverIP, std::string message)
{
  // Initialize parameters from json
  js = jsonString;
  serverAddress = serverIP;
  info = message;
  
  // 'scene' is an autorelease object
  auto scene = Scene::create();
  
  // 'layer' is an autorelease object
  auto layer = BankScene::create();
  
  // add layer as a child to scene
  scene->addChild(layer);
  
  // return the scene
  return scene;
}

// on "init" you need to initialize your instance
bool BankScene::init()
{
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() )
  {
    return false;
  }
  
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  
  // Parse Json
  Json* json = Json_create(js);
  _sessionID = Json_getString(json, "sessionID", "NULL");
  _numOfPlayers = Json_getInt(json, "numOfPlayers", -1);
  _player1 = Json_getString(json, "1", "NULL");
  if (_numOfPlayers >= 2) {
    _player2 = Json_getString(json, "2", "NULL");
  }
  if (_numOfPlayers >= 3) {
    _player3 = Json_getString(json, "3", "NULL");
  }
  if (_numOfPlayers == 4) {
    _player4 = Json_getString(json, "4", "NULL");
  }
  
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

  
  // Transfer Label
  auto transferLabel = Label::createWithTTF("GIAO DỊCH", "fonts/arial.ttf", 20.0f);
  //sessionLabel->setAnchorPoint(Vec2(0.5, 1));
  transferLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                 origin.y + visibleSize.height - transferLabel->getContentSize().height));
  this->addChild(transferLabel);
  
  // Player Slider
  auto slider = ui::Slider::create();
  slider->loadBarTexture("res/Slider_Back.png");
  slider->loadSlidBallTextureNormal("res/SliderNode_Normal.png");
  slider->loadSlidBallTexturePressed("res/SliderNode_Press.png");
  slider->loadSlidBallTextureDisabled("res/SliderNode_Disable.png");
  slider->setPercent(12.5f);
  slider->setPosition(Vec2(origin.x + visibleSize.width / 2,
                           transferLabel->getPositionY() - 40));
  // Enable if there are more than 1 player
  slider->setEnabled(_numOfPlayers > 1);
  slider->addEventListener(CC_CALLBACK_2(BankScene::sliderEvent, this));
  this->addChild(slider);
  
  // Recipent Label
  _recipentLabel = Label::createWithTTF(_player1, "fonts/arial.ttf", 20.0f);
  _recipentLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                  slider->getPosition().y - 40));
  this->addChild(_recipentLabel);
  
  // Set default recipent
  _recipentName = _player1;
  
  // Amount textfield
  auto amountTextField = ui::TextField::create("Số lượng để chuyển", "fonts/arial.ttf", 20.0f);
  amountTextField->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                    _recipentLabel->getPositionY() - 40));
  amountTextField->addEventListener(CC_CALLBACK_2(BankScene::amountInputEvent, this));
  this->addChild(amountTextField);
  
  // Send money button
  _sendBtn = ui::Button::create("res/touchid-64.png");
  _sendBtn->setPosition(Vec2(origin.x + visibleSize.width / 2,
                             amountTextField->getPositionY() - 50));
  _sendBtn->addTouchEventListener(CC_CALLBACK_2(BankScene::sendButtonTapped, this));
  this->addChild(_sendBtn);
  
  // Loading Bar placeholder
  auto loadingPlaceholder = Sprite::create("res/Slider_Back.png");
  loadingPlaceholder->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                       _sendBtn->getPositionY() - 50));
  this->addChild(loadingPlaceholder);
  
  // Loading Bar
  _loadingBar = ui::LoadingBar::create("res/Slider_PressBar.png");
  _loadingBar->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                _sendBtn->getPositionY() - 50));
  _loadingBar->setPercent(0);
  this->addChild(_loadingBar);
  
  // Error Label
  _errorLabel = Label::createWithTTF(info, "fonts/arial.ttf", 20.0f);
  _errorLabel->setTextColor(Color4B::GREEN);
  _errorLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                loadingPlaceholder->getPositionY() - 40));
  this->addChild(_errorLabel);
  
  // Create HTTP Request
  _request = new network::HttpRequest();
  _request->setRequestType(network::HttpRequest::Type::POST);
  _request->setTag("POST bankTransfer");
  _request->setUrl(("http://" + serverAddress + "/monopoly/").c_str());
  _request->setResponseCallback(CC_CALLBACK_2(BankScene::onHttpRequestCompleted, this));
  
  // Display Playertab
  // Create menu tab
  auto playerMenuLabel = Label::createWithTTF("Người Chơi", "fonts/arial.ttf", 20.0f);
  auto playerTab = MenuItemLabel::create(playerMenuLabel);
  playerTab->setEnabled(true);
  playerTab->setCallback(CC_CALLBACK_1(BankScene::playerSceneChange, this));
  
  auto bankLabel = Label::createWithTTF("Ngân Hàng", "fonts/arial.ttf", 20.0f);
  auto bank = MenuItemLabel::create(bankLabel);
  bank->setEnabled(true);
  
  auto menu = Menu::create(playerTab, bank, NULL);
  menu->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 20));
  menu->alignItemsHorizontallyWithPadding(visibleSize.width / 5);
  this->addChild(menu);
  
  return true;
}

// MARK: - Text field input

void BankScene::amountInputEvent(cocos2d::Ref *sender, ui::TextField::EventType type) {
  ui::TextField* field = dynamic_cast<ui::TextField*>(sender);
  switch (type) {
    case ui::TextField::EventType::INSERT_TEXT:
    case ui::TextField::EventType::DELETE_BACKWARD:
      _amount = atoi(field->getString().c_str());
      break;
    case ui::TextField::EventType::ATTACH_WITH_IME:
      _sendBtn->setEnabled(false);
      break;
    case ui::TextField::EventType::DETACH_WITH_IME:
      _sendBtn->setEnabled(true);
      break;
    default:
      break;
  }
}

// MARK: - Button Handler

void BankScene::sendButtonTapped(cocos2d::Ref *sender, ui::Widget::TouchEventType type) {
  switch (type) {
    case ui::Widget::TouchEventType::ENDED:
      _loadingBar->setPercent(5);
      if (_amount > 0) {
        // Execute when amount is entered
        _loadingBar->setPercent(10);
        _plainSendData = "fromBank=true&recipent=" + _recipentName + "&amount=" + std::to_string(_amount) + "&sessionID=" + _sessionID;
        _dataToSend = _plainSendData.c_str();
        _loadingBar->setPercent(20);
        _request->setRequestData(_dataToSend, strlen(_dataToSend));
        network::HttpClient::getInstance()->send(_request);
      } else {
        // Display error
        _loadingBar->setPercent(0);
        _errorLabel->setTextColor(Color4B::RED);
        _errorLabel->setString("Vui lòng nhập đầy đủ thông tin");
        _errorLabel->setVisible(true);
      }
      break;
    default:
      break;
  }
}

// MARK: - Network Handler

void BankScene::onHttpRequestCompleted(network::HttpClient *sender, network::HttpResponse *response) {
  printf("\n");
  // Print connection status
  if (response->getResponseCode() == 200) {
    printf("Connected successfully\n");
    printf("Transfering money from the bank\n");
    
    _loadingBar->setPercent(45);
    std::vector<char> *buffer = response->getResponseData();
    
    // Convert the response data to plain string
    char *concatenated = (char*)malloc(buffer->size());
    std::string string2(buffer->begin(), buffer->end());
    strcpy(concatenated, string2.c_str());
    printf("%s\n", concatenated);
    _loadingBar->setPercent(50);
    
    Json* json = Json_create(concatenated);
    auto errorCode = Json_getInt(json, "errorCode", -99);
    _loadingBar->setPercent(75);
    
    if (errorCode == -1) {
      // No error, switch to next scene
      _loadingBar->setPercent(100);
      auto scene = BankScene::createScene(concatenated, serverAddress, "Xong!");
      auto director = Director::getInstance();
      director->replaceScene(scene);
    } else {
      _loadingBar->setPercent(0);
      // Error from server
      auto message = Json_getString(json, "message", "NULL");
      _errorLabel->setTextColor(Color4B::RED);
      _errorLabel->setString(message);
      _errorLabel->setVisible(true);
      printf("%s\n", message);
    }
  } else {
    // Failed to connect to server
    _loadingBar->setPercent(0);
    _errorLabel->setTextColor(Color4B::RED);
    _errorLabel->setString("Không thể kết nối tới Server\nLỗi: " + std::to_string(response->getResponseCode()));
    _errorLabel->setVisible(true);
    printf("Error: %li", response->getResponseCode());
  }
}

// MARK: - Scene change

void BankScene::playerSceneChange(cocos2d::Ref *sender) {
  // On button tap change to player scene
  auto scene = PlayerScene::createScene(js, serverAddress, "");
  auto director = Director::getInstance();
  director->replaceScene(scene);
}

// MARK: - Slider Event Handler

void BankScene::sliderEvent(cocos2d::Ref *sender, ui::Slider::EventType type) {
  if (type == ui::Slider::EventType::ON_PERCENTAGE_CHANGED) {
    
    // When slider changes value
    // 12.5, 37.5, 62.5 and 87.5 are 4 values on the slider
    // represent 4 players
    ui::Slider *slider = dynamic_cast<ui::Slider*>(sender);
    auto percent = slider->getPercent();
    if (percent <= 12.5f) {
      slider->setPercent(12.5f);
      _recipentLabel->setString(_player1);
      _recipentName = _player1;
    } else if (percent <= 37.5) {
      slider->setPercent(37.5f);
      if (_numOfPlayers >= 2) {
        _recipentLabel->setString(_player2);
        _recipentName = _player2;
      } else {
        _recipentLabel->setString("Không có người chơi");
      }
    } else if (percent <= 62.5) {
      slider->setPercent(62.5f);
      if (_numOfPlayers >= 3) {
        _recipentLabel->setString(_player3);
        _recipentName = _player3;
      } else {
        _recipentLabel->setString("Không có người chơi");
      }
    } else {
      slider->setPercent(87.5f);
      if (_numOfPlayers == 4) {
        _recipentLabel->setString(_player4);
        _recipentName = _player4;
      } else {
        _recipentLabel->setString("Không có người chơi");
      }
    }
  }
}