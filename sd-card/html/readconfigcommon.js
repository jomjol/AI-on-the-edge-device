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

function UpdateConfigFileReferenceChange(_basepath){
     for (var _index = 0; _index < ref.length; ++_index){
          var zeile = ref[_index]["name"] + " " + ref[_index]["x"] + " " + ref[_index]["y"];
          var _pos = ref[_index]["pos_ref"];
          config_split[_pos] = zeile;          
     }

     zeile = "InitialRotate = " + initalrotate["angle"];
     var _pos = initalrotate["pos_config"];
     config_split[_pos] = zeile;

     var mirror = false;
     if (initalrotate.hasOwnProperty("mirror")) {
          mirror = initalrotate["mirror"];
     }
     var mirror_pos = -1;
     if (initalrotate.hasOwnProperty("pos_config_mirror")) {
          mirror_pos = initalrotate["pos_config_mirror"];
     }     
     if (mirror_pos > -1) {
          if (mirror) {
               config_split[mirror_pos] = "InitialMirror = true";
          }
          else {
               config_split[mirror_pos] = "InitialMirror = false";
          }
     }
     else {
          if (mirror) {       // neue Zeile muss an der richtigen Stelle eingefügt werden - hier direct nach [Alignment]
               var aktline = 0;

               while (aktline < config_split.length){
                    if (config_split[aktline].trim() == "[Alignment]") {
                         break;
                    }
                    aktline++
               }

               // fuege neue Zeile in config_split ein
               var zw = config_split[config_split.length-1];
               config_split.push(zw);
               for (var j = config_split.length-2; j > aktline + 1; --j){
                    config_split[j] = config_split[j-1];
               }

               config_split[aktline + 1] = "InitialMirror = True"
          }
     }

     SaveConfigToServer(_basepath);
}

function UpdateConfig(zw, _index, _enhance, _basepath){
     var zeile = zw["name"] + " " + zw["x"] + " " + zw["y"];
     var _pos = ref[_index]["pos_ref"];
     config_split[_pos] = zeile;

     SaveConfigToServer(_basepath);

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



function ZerlegeZeile(input)
     {
          var Output = Array(0);
          delimiter = " =,\r";
     
          input = trim(input, delimiter);
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
     url = _basepath + "/editflow.html?task=copy&in=" + _source + "&out=" + _target;
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