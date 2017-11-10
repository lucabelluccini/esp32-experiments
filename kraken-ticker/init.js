// Connects to the Kraken API and retrieves a given currency pair
// Prints the currency pair, last value, high, low
// Add the kraken.cer content to the ca.pem

load('api_config.js');
load('api_gpio.js');
load('api_net.js');
load('api_sys.js');
load('api_timer.js');
load('api_http.js');

// For DoIt ESP-32 Vroom DevKit, the embedded led pin is not the standard one
let led = 2;
let refresh_interval_seconds = 5;
let pair = 'XBTEUR';
let api_url = 'https://api.kraken.com/0/public/Ticker?pair=' + pair;

// Blink built-in LED every second
GPIO.set_mode(led, GPIO.MODE_OUTPUT);
Timer.set(refresh_interval_seconds * 10000, true /* repeat */, function() {
  GPIO.toggle(led);
  HTTP.query({
    url: api_url,
    headers: { 'Accept': 'application/json' },
    //data: {foo: 1, bar: 'baz'},      
    success: function(body, full_http_msg) {
      print(body);
      let response = JSON.parse(body);
      if(response.error.length === 0)
      {
        let last = response.result.XXBTZEUR.c[0];
        let high = response.result.XXBTZEUR.h[0];
        let low = response.result.XXBTZEUR.l[0];
        print(pair, last, high, low);
      }
    },
    error: function(err) { print('Error', err); },
  });

}, null);

// Monitor network connectivity.
Net.setStatusEventHandler(function(ev, arg) {
  let evs = "???";
  if (ev === Net.STATUS_DISCONNECTED) {
    evs = "DISCONNECTED";
  } else if (ev === Net.STATUS_CONNECTING) {
    evs = "CONNECTING";
  } else if (ev === Net.STATUS_CONNECTED) {
    evs = "CONNECTED";
  } else if (ev === Net.STATUS_GOT_IP) {
    evs = "GOT_IP";
  }
  print("== Net event:", ev, evs);
}, null);
