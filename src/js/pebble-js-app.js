var initialized = false;
var config_url = "http://robertocalderon.ca/pebble/thingbrokerpebbleconfiguration.php?v=0.1";

Pebble.addEventListener("ready", function() {
  console.log("ready called!");
  initialized = true;
});

Pebble.addEventListener("appmessage",
  function(e) {
    console.log(">>> JS Received message: " + e.payload);
    var o = JSON.stringify(e.payload);
    console.log("Source: " + o);
    
    //var thingbrokerurl = e.payload.thingbrokerurl;  TODO: check why it gets corrupted after a bit
    //var thingid = e.payload.thingid; 
    var thingbrokerurl = "http://kimberly.magic.ubc.ca:8080/thingbroker" 
    var thingid = "checkin1396992920"

    console.log(thingbrokerurl);
    console.log(thingid);

    console.log(">>> Posting Event to " + thingbrokerurl + "/things/" + thingid + "/events");
    var xmlhttp = new XMLHttpRequest();   // new HttpRequest instance 
    xmlhttp.open("POST", thingbrokerurl + "/things/" + thingid + "/events");
    xmlhttp.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
    xmlhttp.send(JSON.stringify({thing:"pebble", gesture:"highfive"}));

  }
);

Pebble.addEventListener("showConfiguration", function() {
  console.log("showing configuration");
  var url = config_url;

  for(var i = 0, x = window.localStorage.length; i < x; i++) {
    var key = window.localStorage.key(i);
    var val = window.localStorage.getItem(key);

    if(val != null) {
      url += "&" + encodeURIComponent(key) + "=" + encodeURIComponent(val);
    }
  }
  console.log(url);
  Pebble.openURL(url);  
});

Pebble.addEventListener("webviewclosed", function(e) {
  console.log("configuration closed");

  var options = JSON.parse(decodeURIComponent(e.response));  
  console.log("Options = " + JSON.stringify(options));

  for(key in options) {
    window.localStorage.setItem(key, options[key]);
  }

  Pebble.sendAppMessage(options);
});
