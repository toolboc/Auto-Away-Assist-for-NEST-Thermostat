module.exports = function (context, eventInput) {
    context.log('Triggered!');

    if(eventInput.event === "motion" && eventInput.data === "true");
    {
        var url = 'https://developer-api.nest.com/structures/' + process.env.STRUCTURE_ID + '/away?auth=' + process.env.ACCESS_TOKEN;
        var request = require('request');

        request({ method: 'PUT', url: url, followAllRedirects : true, 'content-type': 'application/json', body: '"home"' }, function (err, res, body) {        
                
                context.log('Away status set to: ' + JSON.parse(body));
                                
                context.done();
            
        });
    }             
}
