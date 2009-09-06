

SiteFusion.Classes.Browser = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULBrowser',
	
	initialize: function( win ) {
		this.element = win.createElement( 'browser' );
		this.element.sfNode = this;
		this.element.setAttribute('disablehistory', true);
		
		this.setEventHost();
	},
	
	setSrc: function( src ) {
		this.widgetElement.setAttribute( 'src', src );
	},
	
	reload: function() {
		this.widgetElement.reload( );
	}
} );


SiteFusion.Classes.PrintBox = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULPrintBox',
	
	initialize: function( win ) {
		this.element = win.createElement( 'browser' );
		this.element.sfNode = this;
		this.element.setAttribute( 'src', 'about:blank' );
		
		this.setEventHost( [ 'ready' ] );
	},
	
	setContent: function( html ) {
		if( ! this.element.contentDocument ) {
			var oThis = this;
			setTimeout( function() { oThis.setContent( html ); }, 10 );
			return;
		}
		
		this.element.contentDocument.open();
		this.element.contentDocument.write( html );
		this.element.contentDocument.close();
	},
	
	print: function( silent ) {
		var wbp = this.element.contentWindow.QueryInterface(Ci.nsIInterfaceRequestor).getInterface(Ci.nsIWebBrowserPrint);
		var oThis = this;
		
		var printProgressListener = {
			done: false,
			onLocationChange: function( prog, req, loc ) {},
			onProgressChange: function( prog, req, selfprog, maxselfprog, totalprog, maxtotalprog ) {
				if( totalprog == maxtotalprog && !this.done ) {
					setTimeout( function() { oThis.fireEvent( 'ready' ); }, 1000 );
					this.done = true;
				}
			},
			onSecurityChange: function( prog, req, state ) {},
			onStateChange: function( prog, req, stateFlags, status ) {},
			onStatusChange: function( prog, req, status, message ) {}
		};
		
		var ps = this.getPrintSettings();
		ps.printSilent = silent;
		ps.headerStrRight = '';
		ps.footerStrLeft = '';
		ps.footerStrRight = '';
		
		try {
			wbp.print( ps, printProgressListener );
		}
		catch ( ex ) {}
	},
	
	setPrinterDefaultsForSelectedPrinter: function (aPrintService, aPrintSettings) {
		if (!aPrintSettings.printerName)
			aPrintSettings.printerName = aPrintService.defaultPrinterName;
		
		// First get any defaults from the printer 
		aPrintService.initPrintSettingsFromPrinter(aPrintSettings.printerName, aPrintSettings);
		// now augment them with any values from last time
		aPrintService.initPrintSettingsFromPrefs(aPrintSettings, true, aPrintSettings.kInitSaveAll);
	},
	
	getPrintSettings: function () {
		var pref = Cc["@mozilla.org/preferences-service;1"].getService(Ci.nsIPrefBranch);
		if (pref) {
			var gPrintSettingsAreGlobal = pref.getBoolPref("print.use_global_printsettings", false);
			var gSavePrintSettings = pref.getBoolPref("print.save_print_settings", false);
		}
		var sys = new System;
		var printSettings;
		try {
			var printService = Cc["@mozilla.org/gfx/printsettings-service;1"].getService(Ci.nsIPrintSettingsService);
			if (gPrintSettingsAreGlobal && sys.os == 'win32') {
				printSettings = printService.globalPrintSettings;
				this.setPrinterDefaultsForSelectedPrinter(printService, printSettings);
			} else {
				printSettings = printService.newPrintSettings;
			}
		} catch (e) {
			SiteFusion.Error("getPrintSettings: "+e+"\n");
		}
		return printSettings;
	}
} );

