const app = getApp();
Page({
  /**
   * 页面的初始数据
   */
  data: {
    type_switch: "关",
    showLoginModal: true,
    password: ''
  },

  onLoad: function () {
          
    if (!app.globalData.isConnected) {
      app.doConnect();
    }
    this.setData({
      showLoginModal: true, // 页面加载时显示登录弹窗
      currentAngle: app.globalData.currentAngle,
      type_switch: app.globalData.type_switch ? "开" : "关",
    });
  },
  
  onPasswordInput: function(e) {
    this.setData({
        password: e.detail.value
    });
},
onLogin: function() {
    const password = this.data.password;
    const correctPassword = "lx520"; // 这里写死的密码

    if (password === correctPassword) {
        wx.showToast({
            title: '登录成功',
            icon: 'success',
            duration: 2000
        });
        this.setData({
            showLoginModal: false
        });
    } else {
        wx.showToast({
            title: '密码错误',
            icon: 'none',
            duration: 2000
        });
    }
},
  // type_switch: function (e) {
  //   const sw = e.detail.value;
  //   const message = sw ? '开' : '关';
  //   this.setData({ type_switch: message });

  //   if (app.globalData.client && app.globalData.isConnected) {
  //     const topic = `/k1gm3ZRZ0Xw/wechat/user/update`; // 替换成你的 MQTT 主题
  //     const payload = sw ? '1' : '0'; // 根据开关状态发送不同的消息
  //     app.globalData.client.publish(topic, payload, (err) => {
  //       if (err) {
  //         console.error('消息发送失败:', err);
  //       } else {
  //         console.log(`消息已发送: ${payload}`);
  //       }
  //     });
  //   } else {
  //     console.error('MQTT 客户端未连接');
  //   }
  // },

  // });


  type_switch: function (e) {
    const sw = e.detail.value;
    const message = sw ? '开' : '关';
    this.setData({ type_switch: message });

    if (app.globalData.client && app.globalData.isConnected) {
      this.sendMqttMessage(app.globalData.currentAngle, sw ? '1' : '0');
      app.globalData.type_switch = sw;
    } else {
      console.error('MQTT 客户端未连接');
    }
  },

  sendMqttMessage: function (angle, status) {
    let payload = '';
    if (angle < 100) {
      payload = '2' + angle.toString().padStart(2, '0') + status;
    } else {
      payload = angle.toString() + status;
    }

    const topic = `/k1gm3ZRZ0Xw/wechat/user/update`; // 替换成你的 MQTT 主题
    app.globalData.client.publish(topic, payload, (err) => {
      if (err) {
        console.error('消息发送失败:', err);
      } else {
        console.log(`消息已发送: ${payload}`);
      }
    });
  },

  adjustLeft: function () {
    app.globalData.currentAngle += 1;
    this.sendMqttMessage(app.globalData.currentAngle, this.data.type_switch === '开' ? '1' : '0');
    this.setData({ currentAngle: app.globalData.currentAngle });
  },

  adjustRight: function () {
    app.globalData.currentAngle -= 1;
    this.sendMqttMessage(app.globalData.currentAngle, this.data.type_switch === '开' ? '1' : '0');
    this.setData({ currentAngle: app.globalData.currentAngle });
  },
  turnLeft: function () {
    const payload = '4900'; // 左转对应的消息
    const topic = `/k1gm3ZRZ0Xw/wechat/user/update`; // 替换成左转的 MQTT 主题
    app.globalData.client.publish(topic, payload, (err) => {
      if (err) {
        console.error('停机消息发送失败:', err);
      } else {
        console.log('停机消息已发送');
      }
    });
  },

  turnRight: function () {
    const payload = '6000'; // 右转对应的消息
    const topic = `/k1gm3ZRZ0Xw/wechat/user/update`; // 替换成右转的 MQTT 主题
    app.globalData.client.publish(topic, payload, (err) => {
      if (err) {
        console.error('右转消息发送失败:', err);
      } else {
        console.log('右转消息已发送');
      }
    });
  },
  /**
   * 生命周期函数--监听页面加载
   */
  onLoad(options) {

  },

  /**
   * 生命周期函数--监听页面初次渲染完成
   */
  onReady() {

  },

  /**
   * 生命周期函数--监听页面显示
   */
  onShow() {

  },

  /**
   * 生命周期函数--监听页面隐藏
   */
  onHide() {

  },

  /**
   * 生命周期函数--监听页面卸载
   */
  onUnload() {

  },

  /**
   * 页面相关事件处理函数--监听用户下拉动作
   */
  onPullDownRefresh() {

  },

  /**
   * 页面上拉触底事件的处理函数
   */
  onReachBottom() {

  },

  /**
   * 用户点击右上角分享
   */
  onShareAppMessage() {

  }
})