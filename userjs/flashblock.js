(function() {

// BlockFlash2 
        // version 0.4 modified to fit haku
        // Released under the GPL license 
        // http://www.gnu.org/copyleft/gpl.html  
        // ---------------------------------------------------------------        
		
		function flashblock() 
		{
			// embed tags
			
			xpath("//embed").forEach(function(embed) // put all embed objects in array and check each
			{            
				if (embed.parentNode.nodeName != "OBJECT" && embed.parentNode.style.display != "none") // handle embeds within objects as objects
				{       
					if(checkforflash(embed))
					{
						add_play_flash_div(embed)
					};
			   };
			});
			
			// object tags
			
			xpath("//object").forEach(function(object) 
			{     
				if(checkforflash(object))
				{
					add_play_flash_div(object)
				};
			});
		}
        
        function checkforflash(potl_item) // checks the element passed to it for Flash content
		{                    
            if (potl_item.hasAttribute("flashvars") )
			{
                return true
            };
        
        
            if (potl_item.hasAttribute("type") && potl_item.getAttribute("type").match(/flash|shockwave/))
			{
                return true
            };
        
            if (potl_item.hasAttribute("src") && potl_item.getAttribute("src").match(/.swf|shockwave|flash|eyewonder/))
			{
                return true
            };
            if (potl_item.innerHTML.match(/.swf|shockwave|flash|eyewonder/)) 
			{
                return true
            };
        
            return false;
        
        };
        
        
        
        
        
        function add_play_flash_div(flash) // places the button-like div before the flash node
		{            
            var placeholder=document.createElement("div");
            savedDisplay = flash.style.display;
        
            placeholder.setAttribute("class", "BlockFlash2");
        
            flash.parentNode.insertBefore(placeholder, flash);  
        
            flash.style.display='none';                // hides the Flash node
            flash.on=false;
        
            placeholder.style.cursor='pointer';
            placeholder.style.background='orange';     // don't like orange buttons? Change color here.
            placeholder.style.textAlign='center';
            placeholder.style.textTransform='capitalize';
            placeholder.style.color='black';
            placeholder.style.fontSize='10px';
            placeholder.innerHTML="[Play Flash]";
        
			
            placeholder.addEventListener( 'click',     // the on/off switch
        
                function() {
        
                    placeholder=this;
        
                    flash=this.nextSibling;            // acts on the Flash-containing node following the div
        
                        if (flash.on) {
        
                            flash.style.display='none';
                            flash.style.visibility = 'hidden';
        
                            placeholder.innerHTML="[Play Flash]";
        
                            flash.on=false;
        
                    } else {
        
                            flash.style.display=savedDisplay;  // reveals the Flash node
                         flash.style.visibility='visible';
        
                            placeholder.innerHTML="[Stop Flash]";
        
                            flash.on=true;
        
                    }
        
                },
        
                true
        
            );
        
            return true;
        
        }
        
        function xpath (p, context) 
		{
            if (!context) context = document;
        
            var i, arr = [], xpr = document.evaluate(p, context, null, XPathResult.UNORDERED_NODE_SNAPSHOT_TYPE, null);
        
            for (i = 0; item = xpr.snapshotItem(i); i++) arr.push(item);
        
            return arr;
        
        };
		
		window.addEventListener('load', flashblock);    
		
})();
