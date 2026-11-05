const sendmail = require('sendmail')({
    silent: false,
    devPort: 1025,
    devHost: 'localhost'
});

const sender_email = 'aanlteixpeoyvln@gmail.com';

function send(messageString) {
    sendmail({
        from: 'vladimir_smelov@gmail.com',
        to: sender_email,
        subject: 'ЛР',
        text: messageString,
    }, function(err, reply) {
        if (err)
        {
            console.error('Ошибка при отправке письма:', err);
        }
        else
        {
            console.log('Письмо успешно отправлено, получатель: ', sender_email);
            console.dir(reply);
        }
    });
}

module.exports = {
    send: send
};