const app = getApp();

Page({
  data: {
    indicatorDots: true,
    autoplay: true,
    interval: 5000,
    duration: 1000,
    type_switch: "关",
    mqttMessages: [], // 显示MQTT消息的数组
    boardcast: [], // 播报消息数组
    token: '', // 百度的token
    voiceBroadcastEnabled: false, // 语音播报开关
    currentMessageIndex: 0 // 当前播放的消息索引
  },
  
  onLoad: function () {
    if (app.globalData.isConnected) {
      this.subscribeToTopic();
    } else {
      app.doConnect();
      const interval = setInterval(() => {
        if (app.globalData.isConnected) {
          clearInterval(interval);
          this.subscribeToTopic();
        }
      }, 1000);
    }
    
    // 获取百度token
    const appkey = 's68KJHz3EZFbLDfXq1044Lm4'; // 百度应用的 一个月过期
    const selKey = 'mN9Nw4un6Y6ECxz9jxBhDi112oQ7pmPY'; // 百度应用的 一个月过期
    wx.request({
      method: 'GET',
      url: `https://openapi.baidu.com/oauth/2.0/token?grant_type=client_credentials&client_id=${appkey}&client_secret=${selKey}`,
      header: {
        'content-type': 'application/json' // 默认值
      },
      success: (res) => {
        this.setData({
          token: res.data.access_token
        });
        this.playWelcomeMessage();
      }
    });

    // 设置语音播报定时器
    this.setVoiceBroadcastInterval();
  },
  
  setVoiceBroadcastInterval: function () {
    setInterval(() => {
      if (this.data.voiceBroadcastEnabled && this.data.boardcast.length > 0) {
        const message = this.data.boardcast[this.data.currentMessageIndex];
        this.playVoiceMessage(message);
        this.setData({
          currentMessageIndex: (this.data.currentMessageIndex + 1) % this.data.boardcast.length
        });
      }
    }, 5000); // 每隔5秒播放一条消息
  },

  playVoiceMessage: function (message) {
    const innerAudioContext = wx.createInnerAudioContext();
    innerAudioContext.src = 'https://tsn.baidu.com/text2audio?lan=zh&ctp=1&cuid=abcdxxx&tok=' + this.data.token + '&tex=' + encodeURIComponent(message) + '&vol=5&per=0&spd=5&pit=5&aue=3';
    innerAudioContext.play();
  },

  playWelcomeMessage: function() {
    const innerAudioContext = wx.createInnerAudioContext();
    innerAudioContext.obeyMuteSwitch = false;
    innerAudioContext.autoplay = true;
    innerAudioContext.src = 'https://tsn.baidu.com/text2audio?lan=zh&ctp=1&cuid=abcdxxx&tok=' + this.data.token + '&tex=' + encodeURIComponent('欢迎使用基于龙芯2K1000LA主控的智能化自动激光除草设备客户端') + '&vol=5&per=0&spd=5&pit=5&aue=3';
    innerAudioContext.onPlay(() => {
      console.log('欢迎信息开始播放');
    });
    innerAudioContext.onError((res) => {
      console.log(res.errMsg);
      console.log(res.errCode);
    });
  },

  switchChange: function (e) {
    this.setData({
      voiceBroadcastEnabled: e.detail.value
    });
  },

  subscribeToTopic: function () {
    const client = app.globalData.client;
    const topic = '/k1gm3ZRZ0Xw/wechat/user/get'; // 替换成你的订阅主题

    client.subscribe(topic, (err) => {
      if (!err) {
        console.log('订阅成功');
      } else {
        console.error('订阅失败:', err);
      }
    });

    client.on('message', (topic, message) => {
      const currentTime = new Date().toLocaleTimeString();
      const payload = message.toString(); // 将消息转换为字符串

      // 提取舵机角度
      const newAngle = parseInt(payload.slice(0, 3), 10);
      let formattedMessage = `时间：${currentTime}\n消息：`;
      let boardcastmessage = '';

      if (app.globalData.lastAngle !== null && newAngle !== app.globalData.lastAngle) {
        if (newAngle > app.globalData.lastAngle) {
          formattedMessage += "\n当前杂草位置相对激光器偏右，已自动向右跟随";
          boardcastmessage = "发现目标，自动向右跟随";
        } else {
          formattedMessage += "\n当前杂草位置相对激光器偏左，已自动向左跟随";
          boardcastmessage = "发现目标，自动向左跟随";
        }
      }

      // 更新舵机角度
      app.globalData.lastAngle = newAngle;
      app.globalData.currentAngle = newAngle;

      // 更新界面上显示的消息数组
      const newMessages = this.data.mqttMessages.concat(formattedMessage);
      this.setData({
        mqttMessages: newMessages
      });

      // 仅在boardcastmessage不为空时更新播报消息数组
      if (boardcastmessage) {
        const newBoardcast = this.data.boardcast.concat(boardcastmessage);
        this.setData({
          boardcast: newBoardcast
        });
      }
    });
  }
});

