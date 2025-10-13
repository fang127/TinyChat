const config_module = require('./config');
const redis = require('ioredis');

// 创建Redis客户端
const redisClient = new redis({
    host: config_module.redis_host,          // Redis地址
    port: config_module.redis_port,          // Redis端口
    password: config_module.redis_passwd,    // Redis密码
});

// 监听连接是否成功
redisClient.on('error', function (err) {
    console.log('RedisCli connect error');
    redisClient.quit();
})

/**
 * 根据key获取value
 * @param {*} key 
 * @returns 
 */
async function getRedis(key) {
    try {
        const result = await redisClient.get(key)
        if (result === null) {
            console.log('result:', '<' + result + '>', 'This key cannot be find...')
            return null
        }
        console.log('Result:', '<' + result + '>', 'Get key success!...');
        return result
    } catch (error) {
        console.log('GetRedis error is', error);
        return null
    }
}

/**
 * 根据key查询redis中是否存在key
 * @param {*} key 
 * @returns 
 */
async function queryRedis(key) {
    try {
        const result = await redisClient.exists(key)
        //  判断该值是否为空 如果为空返回null
        if (result === 0) {
            console.log('result:<', '<' + result + '>', 'This key is null...');
            return null
        }
        console.log('Result:', '<' + result + '>', 'With this value!...');
        return result
    } catch (error) {
        console.log('QueryRedis error is', error);
        return null
    }
}

/**
 * 设置key和value，并过期时间
 * @param {*} key 
 * @param {*} value 
 * @param {*} exptime 
 * @returns 
 */
async function setRedisExpire(key, value, exptime) {
    try {
        // 设置键和值
        await redisClient.set(key, value)
        // 设置过期时间（以秒为单位）
        await redisClient.expire(key, exptime);
        return true;
    } catch (error) {
        console.log('SetRedisExpire error is', error);
        return false;
    }
}

/**
 * 退出函数
 */
function quit() {
    redisClient.quit();
}

module.exports = { getRedis, queryRedis, quit, setRedisExpire, }