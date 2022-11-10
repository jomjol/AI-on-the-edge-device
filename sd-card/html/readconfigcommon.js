function readconfig_Version(){
     return "1.0.0 - 20200910";
 }

function SaveConfigToServer(_basepath){
     // leere Zeilen am Ende löschen
     var zw = config_split.length - 1;
     while (config_split[zw] == "") {
          config_split.pop();
     }

     var config_gesamt = "";
     for (var i = 0; i < config_split.length; ++i)
     {
          config_gesamt = config_gesamt + config_split[i] + "\n";
     } 

     FileDeleteOnServer("/config/config.ini", _basepath);

     FileSendContent(config_gesamt, "/config/config.ini", _basepath);          
}

function UpdateConfig(zw, _index, _enhance, _basepath){
     var namezw = zw["name"];
     FileCopyOnServer("/img_tmp/ref_zw.jpg", namezw, _basepath);
     var namezw = zw["name"].replace(".jpg", "_org.jpg");
     FileCopyOnServer("/img_tmp/ref_zw_org.jpg", namezw, _basepath);     
}


function createReader(file) {
     var image = new Image();
     reader.onload = function(evt) {
         var image = new Image();
         image.onload = function(evt) {
             var width = this.width;
             var height = this.height;
             alert (width); // will produce something like 198
         };
         image.src = evt.target.result; 
     };
     reader.readAsDataURL(file);
 }



function ZerlegeZeile(input, delimiter = " =\t\r")
     {
          var Output = Array(0);
//          delimiter = " =,\t";
     
          input = trim(input, delimiter);

          /* The input can have multiple formats: 
           *  - key = value
           *  - key = value1 value2 value3 ...
           *  - key value1 value2 value3 ...
           *  
           * Examples:
           *  - ImageSize = VGA
           *  - IO0 = input disabled 10 false false 
           *  - main.dig1 28 144 55 100 false
           * 
           * This causes issues eg. if a password key has a whitespace or equal sign in its value.
           * As a workaround and to not break any legacy usage, we enforce to only use the
           * equal sign, if the key is "password"
           */
          if (input.includes("password")) { // Line contains a password, use the equal sign as the only delimiter and only split on first occurrence
               console.log(input);
               var pos = input.indexOf("=");
               Output.push(trim(input.substr(0, pos), delimiter));
               Output.push(trim(input.substr(pos +1, input.length), delimiter));
          }
          else { // Legacy Mode
               var pos = findDelimiterPos(input, delimiter);
               var token;
               while (pos > -1) {
                    token = input.substr(0, pos);
                    token = trim(token, delimiter);
                    Output.push(token);
                    input = input.substr(pos+1, input.length);
                    input = trim(input, delimiter);
                    pos = findDelimiterPos(input, delimiter);
               }
               Output.push(input);
          }
     
          return Output;
     
     }    

function findDelimiterPos(input, delimiter)
     {
          var pos = -1;
          var zw;
          var akt_del;
     
          for (var anz = 0; anz < delimiter.length; ++anz)
          {
               akt_del = delimiter[anz];
               zw = input.indexOf(akt_del);
               if (zw > -1)
               {
                    if (pos > -1)
                    {
                         if (zw < pos)
                              pos = zw;
                    }
                    else
                         pos = zw;
               }
          }
          return pos;
     }
     
     

function trim(istring, adddelimiter)
     {
          while ((istring.length > 0) && (adddelimiter.indexOf(istring[0]) >= 0)){
               istring = istring.substr(1, istring.length-1);
          }
          
          while ((istring.length > 0) && (adddelimiter.indexOf(istring[istring.length-1]) >= 0)){
               istring = istring.substr(0, istring.length-1);
          }

          return istring;
     }
     

function getConfig()
{
     return config_gesamt;
}

     
function loadConfig(_basepath) {
     var xhttp = new XMLHttpRequest();
     try {
          url = _basepath + '/fileserver/config/config.ini';     
          xhttp.open("GET", url, false);
          xhttp.send();
          config_gesamt = xhttp.responseText;
          config_gesamt = config_gesamt.replace("InitalRotate", "InitialRotate");         // Korrigiere Schreibfehler in config.ini !!!!!
     }
     catch (error)
     {
     //          alert("Deleting Config.ini failed");
     }
     return true;
}

     


function dataURLtoBlob(dataurl) {
     var arr = dataurl.split(','), mime = arr[0].match(/:(.*?);/)[1],
          bstr = atob(arr[1]), n = bstr.length, u8arr = new Uint8Array(n);
     while(n--){
          u8arr[n] = bstr.charCodeAt(n);
     }
     return new Blob([u8arr], {type:mime});
     }	
     
function FileCopyOnServer(_source, _target, _basepath = ""){
     url = _basepath + "/editflow?task=copy&in=" + _source + "&out=" + _target;
     var xhttp = new XMLHttpRequest();  
     try {
          xhttp.open("GET", url, false);
          xhttp.send();     }
     catch (error)
     {
//          alert("Deleting Config.ini failed");
     }
}

function FileDeleteOnServer(_filename, _basepath = ""){
     var xhttp = new XMLHttpRequest();
     var okay = false;

     xhttp.onreadystatechange = function() {
          if (xhttp.readyState == 4) {
               if (xhttp.status == 200) {
                    okay = true;
               } else if (xhttp.status == 0) {
//                    alert("Server closed the connection on delete abruptly!");
//                    location.reload()
               } else {
//                    alert(xhttp.status + " Error!\n" + xhttp.responseText);
//                    location.reload()
               }
          }
     };
     try {
          var url = _basepath + "/delete" + _filename;
          xhttp.open("POST", url, false);
          xhttp.send();
     }
     catch (error)
     {
//          alert("Deleting Config.ini failed");
     }

     return okay;
}

function FileSendContent(_content, _filename, _basepath = ""){
     var xhttp = new XMLHttpRequest();  
     var okay = false;

     xhttp.onreadystatechange = function() {
          if (xhttp.readyState == 4) {
               if (xhttp.status == 200) {
                    okay = true;
               } else if (xhttp.status == 0) {
                    alert("Server closed the connection abruptly!");
               } else {
                    alert(xhttp.status + " Error!\n" + xhttp.responseText);
               }
          }
     };

     try {
          upload_path = _basepath + "/upload" + _filename;
          xhttp.open("POST", upload_path, false);
          xhttp.send(_content);
     }
     catch (error)
     {
//          alert("Deleting Config.ini failed");
     }     
    return okay;        
}


function SaveCanvasToImage(_canvas, _filename, _delete = true, _basepath = ""){
     var JPEG_QUALITY=0.8;
     var dataUrl = _canvas.toDataURL('image/jpeg', JPEG_QUALITY);	
     var rtn = dataURLtoBlob(dataUrl);

     if (_delete) {
          FileDeleteOnServer(_filename, _basepath);
     }
	
     FileSendContent(rtn, _filename, _basepath);
}

function MakeContrastImageZW(zw, _enhance, _basepath){
     _filename = zw["name"].replace("/config/", "/img_tmp/");
     url = _basepath + "/editflow?task=cutref&in=/config/reference.jpg&out=" + _filename + "&x=" + zw["x"] + "&y="  + zw["y"] + "&dx=" + zw["dx"] + "&dy=" + zw["dy"];
     if (_enhance == true){
          url = url + "&enhance=true";
     }

     var xhttp = new XMLHttpRequest();  
     try {
          xhttp.open("GET", url, false);
          xhttp.send();     }
     catch (error)
     {
//          alert("Deleting Config.ini failed");
     }
}



function MakeRefZW(zw, _basepath){
     _filetarget = zw["name"].replace("/config/", "/img_tmp/");
     _filetarget = _filetarget.replace(".jpg", "_org.jpg");
     url = _basepath + "/editflow?task=cutref&in=/config/reference.jpg&out="+_filetarget+"&x=" + zw["x"] + "&y="  + zw["y"] + "&dx=" + zw["dx"] + "&dy=" + zw["dy"];
     var xhttp = new XMLHttpRequest();  
     try {
          xhttp.open("GET", url, false);
          xhttp.send();     }
     catch (error)
     {
//          alert("Deleting Config.ini failed");
     }
     _filetarget2 = zw["name"].replace("/config/", "/img_tmp/");
//     _filetarget2 = _filetarget2.replace(".jpg", "_org.jpg");
     FileCopyOnServer(_filetarget, _filetarget2, _basepath);
}

