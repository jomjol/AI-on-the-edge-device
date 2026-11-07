 
/* The UI can also be run locally, but you have to set the IP of your device accordingly.
 * And you also might have to disable CORS in your webbrowser!
 * Eg using https://chromewebstore.google.com/detail/allow-cors-access-control/lhobafahddgcelffkeicbaginigeejlf?utm_source=ext_app_menu on chrome
 * Keep empty to disable using it. Enabling it will break access through a forwared port, see 
 * https://github.com/jomjol/AI-on-the-edge-device/issues/2681 */
var domainname_for_testing = "";
//var domainname_for_testing = "192.168.1.151";


/* Returns the domainname with prepended protocol.
Eg. http://watermeter.fritz.box or http://192.168.1.5 */
function getDomainname(){
    var host = window.location.hostname;
    if (domainname_for_testing != "") {
        console.log("Using pre-defined domainname for testing: " + domainname_for_testing);
        domainname = "http://" + domainname_for_testing
    }
    else
    {
        domainname = window.location.protocol + "//" + host;
        if (window.location.port != "") {
            domainname = domainname + ":" + window.location.port;
        }
    }

    return domainname;
}

function UpdatePage(_dosession = true){
    var zw = location.href;
    zw = zw.substr(0, zw.indexOf("?"));
    if (_dosession) {
        window.location = zw + '?session=' + Math.floor((Math.random() * 1000000) + 1); 
    }
    else {
        window.location = zw; 
    }
}

        
var fwVersion = "";
var webUiVersion = "";

/* Fills in the hostname and checks that the firmware and the web interface are on the same version. */
function LoadDeviceInfo() {
    _domainname = getDomainname(); 

    var xhttp = new XMLHttpRequest();
    xhttp.addEventListener('load', function(event) {
        if (xhttp.status >= 200 && xhttp.status < 300) {
            var info = JSON.parse(xhttp.responseText);

            hostname = info.system.hostname;
            document.title = hostname + " - AI on the edge";
            document.getElementById("id_title").innerHTML  = "Digitizer - AI on the edge - " + hostname;

            fwVersion = info.firmware.version;
            document.getElementById("Version").innerHTML  = fwVersion;
            console.log(fwVersion);

            webUiVersion = info.web_ui.version;
            console.log("Web UI Version: " + webUiVersion);

            compareVersions();
        } 
        else {
            console.warn(xhttp.statusText, xhttp.responseText);
            fwVersion = "NaN";
            webUiVersion = "NaN";

            // Firmware older than this web interface has no /api/v1, so compareVersions() can't report it
            firework.launch("Could not read the device info (HTTP " + xhttp.status + 
                ")! The firmware might be older than the web interface.", 'warning', 30000);
        }
    });

    try {
        url = _domainname + '/api/v1/info';     
        xhttp.open("GET", url, true);
        xhttp.send();
    }
    catch (error) {
        fwVersion = "NaN";
        webUiVersion = "NaN";
    }
}


function compareVersions() {
    if (fwVersion == "" || webUiVersion == "") {
        return;
    }

    arr = fwVersion.split(" ");
    fWGitHash = arr[arr.length - 1].substring(0, 7);
    arr = webUiVersion.split(" ");
    webUiHash = arr[arr.length - 1].substring(0, 7);
    console.log("FW Hash: " + fWGitHash + ", Web UI Hash: " + webUiHash);
    
    if (fWGitHash != webUiHash) {
        firework.launch("The version of the web interface (" + webUiHash + 
            ") does not match the firmware version (" + 
            fWGitHash + ")! It is suggested to keep them on the same version!", 'warning', 30000);
    }
}
