/**
 * Firework displays short notifications at top of page,
 * then fades out a few seconds later (no user interaction)
 * Source: https://www.jqueryscript.net/other/Simple-Top-Notification-Plugin-with-jQuery-firework-js.html
 *         https://github.com/smalldogs/fireworkjs
 * @param   m   string    message
 * @param   t   string    (optional) message type ('success', 'danger')
 * @param   l   number    (optional) length of time to display message in milliseconds
 */
 ;(function ($, window) {
  "use strict";

  window.firework = {
    launch: function(m, t, l) {
      if (typeof m != 'string') {
        console.error('Error: Call to firework() without a message');
        return false
      }

      var c = 'firework' // css class(es)
        , p = 10 // pixels from top or page to display
        , d = new Date()
        , s = d.getTime() // used to create unique element ids
        , fid = "firework-"+ s; // firework id

      if (typeof t !== 'undefined') c += ' '+ t; // add any user defined classes

      $('.firework').each(function(){ // account for existing fireworks and move new one below
        p += parseInt($(this).height()) + 30
      });

      $('<div id="'+ fid +'" class="'+ c +'">'+ m +'<a onclick="firework.remove(\'#'+ fid +'\')"><img style="height:28px;" src=close.png></a></div>')
        .appendTo('body')
        .animate({
          opacity: 1,
          top: p +'px'
        });

      setTimeout(function(){ firework.remove("#"+ fid) }, typeof l == "number" ? l : 1500);
    },

    remove : function(t) {
      $(t)
        .animate({
          opacity: 0
        })
        .promise()
        .done(function(){
          $(t).remove()
        })
    },

    sticky : function(m, t, l) {
      $.cookie("firework", '{ "message" : "'+ m +'", "type" : "'+ t +'", "display" : "'+ l +'" }', { path: '/' })
    }
  };

  // checks for firework cookie on dom ready
  $(function() {
    if (typeof $.cookie == "function") {
      if ($.cookie("firework")) {
        var ex = $.parseJSON($.cookie("firework"))
        setTimeout(function(){ firework.launch(ex.message, ex.type, parseInt(ex.display) > 0 ? parseInt(ex.display) : null) }, 1000)
        $.cookie("firework", null, { path: '/'})
      }
    }
  });
})(jQuery, window);
