function init() {
    var app = new Vue({
        el: '#esp32',
        data: {
            functionAppName: '',
            reportedTwin: {},
            deviceId:'ESP32',
            lastUpdated: '',
            stateUpdateEndpoint: 'about:blank',
            loading: false,
            restarting: false,
            restarted: false,
            functionAppNameSet: false
        },
        computed: {
        },
        methods: {
            deviceRestart: deviceRestart,
            getTwin: getTwin,
            getState: getState
        }
    });
}

function getTwin() {
    var scope = this;
    var timespan = new Date().getTime();
    var url = `https://${this.functionAppName}.azurewebsites.net/api/esp32-state?action=get&t=${timespan}`;
    this.$http.jsonp(url).then(function(data){
        return data.json();
    }).then(function (data){
        scope.loading = false;
        scope.reportedTwin = data;
        scope.lastUpdated = new Date(scope.reportedTwin.$metadata.$lastUpdated).toLocaleString('en-GB', {hour12: false}).replace(',', '');
    });
}

function getState() {
    if (!this.functionAppName)
    {
        alert('No function name found.');
        return;
    }
    this.functionAppNameSet = true;
    this.loading = true;
    setInterval(this.getTwin, 5000);
}

function deviceRestart(){
    var url = `https://${this.functionAppName}.azurewebsites.net/api/esp32-state?action=restart`;
    this.$http.jsonp(url);
}


init();
