const grpc = require('@grpc/grpc-js')
const messageProto = require('./proto')
const constModule = require('./const')
const emailModule = require('./email')
const { v4: uuidv4 } = require('uuid');
const redisModule = require('./redis');

async function GetVerifyCode(call, callback) {
    console.log("email is ", call.request.email)
    try {
        // 查询redis中是否存在该邮箱
        let queryRes = await redisModule.getRedis(constModule.codePrefix + call.request.email);
        console.log("query res is ", queryRes)
        let uniqueId = queryRes
        if (queryRes == null) {
            // 如果不存在则生成新的验证码
            uniqueId = uuidv4();
            // 取uuid的前四位作为验证码
            if (uniqueId.length > 4) {
                uniqueId = uniqueId.substring(0, 4);
            }
            // 将验证码存入redis中，并设置过期时间为10分钟
            let bres = await redisModule.setRedisExpire(constModule.codePrefix + call.request.email, uniqueId, 600)
            // 如果存入失败则返回错误
            if (!bres) {
                callback(null, {
                    email: call.request.email,
                    error: constModule.Errors.RedisErr
                });
                return;
            }
        }
        console.log("uniqueId is ", uniqueId)
        let textStr = '您的验证码为' + uniqueId + '请三分钟内完成注册'
        // 发送邮件
        let mailOptions = {
            from: 'fhn19591513603@163.com',
            to: call.request.email,
            subject: '验证码',
            text: textStr,
        };

        let sendRes = await emailModule.sendMail(mailOptions);
        console.log("send res is ", sendRes)

        if (!sendRes) {
            callback(null, {
                email: call.request.email,
                error: constModule.Errors.Exception
            });
            return;
        }

        callback(null, {
            email: call.request.email,
            error: constModule.Errors.Success
        });

    } catch (error) {
        console.log("catch error is ", error)

        callback(null, {
            email: call.request.email,
            error: constModule.Errors.Exception
        });
    }

}

function main() {
    var server = new grpc.Server()
    server.addService(messageProto.VerifyService.service, { GetVerifyCode: GetVerifyCode })
    server.bindAsync('0.0.0.0:50051', grpc.ServerCredentials.createInsecure(), () => {
        server.start()
        console.log('grpc server started')
    })
}

main()