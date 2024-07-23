const mqtt = require('/utils/mqtt.min.js');
const crypto = require('/utils/hex_hmac_sha1.js'); //根据自己存放的路径修改
App({
  onLaunch: function () {
    // 注意：这里在程序运行后会直接进行连接，如果你要真机调试，
    // 记得关掉模拟器或者使用一个按钮来控制连接，以避免模拟器和真机同时进行连接导致两边都频繁断线重连！
    this.doConnect();
  },
  doConnect() {
    const deviceConfig = {
      productKey: "k1gm3ZRZ0Xw",
      deviceName: "wechat",
      deviceSecret: "8389dcb2348bca6475c05064b051cff0", // 替换为实际的 deviceSecret
      regionId: "cn-shanghai" // 根据自己的区域替换
    };
    const options = this.initMqttOptions(deviceConfig);
    console.log(options);
    // 替换productKey为你自己的产品的（注意这里是wxs，不是wss，否则你可能会碰到ws不是构造函数的错误）
    const client = mqtt.connect('wxs://k1gm3ZRZ0Xw.iot-as-mqtt.cn-shanghai.aliyuncs.com', options);

    client.on('connect', () => {
      console.log('连接服务器成功');
      // 注意：订阅主题，替换productKey和deviceName
      this.globalData.client = client;
      this.globalData.isConnected = true;
      client.on('error', (error) => {
        console.error('连接服务器失败:', error);
      });
    });
  },

initMqttOptions(deviceConfig) {
  const params = {
    productKey: deviceConfig.productKey,
    deviceName: deviceConfig.deviceName,
    timestamp: Date.now(),
    clientId: Math.random().toString(36).substr(2)
  };
  // CONNECT参数
  const options = {
    keepalive: 60, // 60s
    clean: true, // cleanSession不保持持久会话
    protocolVersion: 4 // MQTT v3.1.1
  };
  // 1.生成clientId，username，password
  options.password = `c362c2f6f65da6376dd207de9b6005c3d5fb2991b779da754714ffc84801c9f1`;
  options.clientId = `k1gm3ZRZ0Xw.wechat|securemode=2,signmethod=hmacsha256,timestamp=1719945648176|`;
  options.username = `wechat&k1gm3ZRZ0Xw`;
  return options;
},
  globalData: {
    client: null,
    isConnected: false,
    lastAngle: 90,
    currentAngle: 90,
    userInfo: null
  }
})
