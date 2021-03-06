<?php
// - - - - - - - - - - - - - BEGIN LICENSE BLOCK - - - - - - - - - - - - -
// Version: MPL 1.1/GPL 2.0/LGPL 2.1
//
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
//
// The Original Code is sitefusion.sourceforge.net code.
//
// The Initial Developer of the Original Code is
// FrontDoor Media Group.
// Portions created by the Initial Developer are Copyright (C) 2009
// the Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Nikki Auburger <nikki@thefrontdoor.nl> (original author)
//   Tom Peeters <tom@thefrontdoor.nl>
//
// - - - - - - - - - - - - - - END LICENSE BLOCK - - - - - - - - - - - - -


/**
 * @package API
 * @subpackage Windows
*/



/**
 * Basic window functionality
 *
 * Not to be constructed directly
 * @package API
 * @subpackage Windows
*/
class XULBasicWindow extends BranchNode {

    public static $STATE_MAXIMIZED = 1;
    public static $STATE_MINIMIZED = 2;
    public static $STATE_NORMAL = 3;
    public static $STATE_FULLSCREEN = 4;

    /**
     * The current state of the window
     * @var integer
     */
    public $windowState;
    public $preventClose    = FALSE;

    public function __construct() {
        $this->windowState = XULBasicWindow::$STATE_NORMAL;
        $this->setEventHandler('sizemodechange', $this, 'onWindowStateChanged');
    }

    /**
     * [INTERNAL FUNCTION] Handles the sizemodechange event
     */
    public function onWindowStateChanged($e, $newState) {
        $this->windowState = $newState;
    }

    /**
     * Prevents the window from closing
     * @param  bool $bool
     */
    public function preventClose($bool = TRUE) {
        $this->preventClose = $bool;
        $this->setProperty( 'preventClose', (bool) $bool );
    }

    /**
     * Sets the width and height of the window
     * @param  int $width  Height
     * @param  int $height Width
     */
    public function size($width = NULL, $height = NULL) {
        if ($width && $height) {
            $this->callMethod( 'windowObject.resizeTo', array($width, $height) );
        }
    }

    /**
     * Moves the window to the specified point
     * @param integer $x
     * @param integer $y
     */
    public function position($x, $y) {
        $this->callMethod( 'windowObject.moveTo', array((int) $x, (int) $y) );
    }

    /**
     * Shows an alert dialog
     * @param  string $msg The message to show
     */
    public function alert($msg) {
        $this->callMethod( 'alert', array($msg) );
    }

    /**
     * Removes the focus from the window
     */
    public function blur() {
        $this->callMethod( 'windowObject.blur' );
    }

    /**
     * Assigns focus to the window
     */
    public function focus() {
        $this->callMethod( 'windowObject.focus' );
    }

    /**
     * Sizes the window according to its content
     */
    public function sizeToContent() {
        $this->callMethod( 'sizeToContent' );
    }

    /**
     * Maximizes the window
     */
    public function maximize() {
        $this->callMethod( 'maximize' );
    }

    /**
     * Minimizes the window
     */
    public function minimize() {
        $this->callMethod( 'minimize' );
    }

    /**
     * Sets the fullscreen state of the window
     * @param  bool $state
     */
    public function fullScreen($state = TRUE) {
        $this->callMethod( 'fullScreen', array((bool) $state));
    }

    public function addStyleSheetRule() {
        $rules = func_get_args();

        for ($n = 0; $n < count($rules); $n++) {
            if (is_array($rules[$n])) {
                array_splice( $rules, $n, 1, $rules[$n] );
                $n--;
                continue;
            } elseif (is_string($rules[$n])) {
                $this->callMethod( 'addStyleSheetRule', array( $rules[$n] ) );
            }
        }
    }

    /**
     * Opens an URL with the client's default browser
     * @param  string $url The URL to open
     */
    public function openLink($url) {
        $this->callMethod( 'openLink', array($url) );
    }

    /**
     * Opens the specified URL in the current Window
     * @param  string $url     The URL to open
     * @param  string $options The options
     */
    public function openUrlWindow($url, $options = 'chrome') {
        $this->callMethod( 'openUrlWindow', array($url, $options) );
    }

    /**
     * Opens the specified URL in a new dialog
     * @param  string $url     The URL to open
     * @param  string $options The options
     */
    public function openUrlDialog($url, $options = 'chrome') {
        $this->callMethod( 'openUrlDialog', array($url, $options) );
    }
}


/**
 * Window element
 *
 * This is the root window class. It is constructed automatically once upon application launch,
 * and should not be constructed separately. Use XULC hildWindow or XULDialog for additional
 * application windows.
 *
 * @see XULChildWindow
 * @see XULDialog
 * @link https://developer.mozilla.org/en/XUL/window
 * @package API
 * @subpackage Windows
*/
class XULWindow extends XULBasicWindow {

    public $hasClosed           = FALSE;
    public $application;
    public $operatingSystem     = '';
    public $workingDirectory    = '';

    public function __construct() {
        $this->hostWindow = $this;
        parent::__construct();
    }

    public function attach() {
        $this->parent = NULL;
    }

    public function open() {
        $this->callMethod( 'windowObject.focus' );
    }

    public function close() {
        $this->application->onWindowClose();

        $this->callMethod( 'windowObject.close' );
    }

    public function title($title) {
        $this->callMethod( 'setTitle', array($title) );
    }
}


/**
 * Child window element
 *
 * @link https://developer.mozilla.org/en/XUL/window
 * @package API
 * @subpackage Windows
*/

class XULChildWindow extends XULBasicWindow {

    public $alwaysLowered       = FALSE;
    public $alwaysRaised        = FALSE;
    public $modal               = FALSE;
    public $centerscreen        = FALSE;
    public $dependent           = TRUE;
    public $dialog              = FALSE;
    public $resizable           = TRUE;
    public $title               = 'SiteFusion ChildWindow';
    public $hasClosed           = FALSE;
    public $remoteConstructor   = 'ChildWindow';

    public function __construct() {
        if (func_num_args()) {
            $args = func_get_args();

            if (count($args) && is_string($args[0])) {
                $this->title( array_shift($args) );
            }
            if (count($args) && is_string($args[0])) {
                $this->buttons = array_shift($args);
            }
            if (count($args) && is_bool($args[0])) {
                $this->centerscreen = array_shift($args);
            }
            if (count($args) && is_bool($args[0])) {
                $this->dependent = array_shift($args);
            }

            if (count($args)) {
                $this->initChildren = $args;
            }
        }

        parent::__construct();
    }

    public function attach() {
        $this->hasClosed = FALSE;
        $this->hostWindow = $this;

        $this->createRemoteObject();
        $this->setProperty( 'alwaysLowered', $this->alwaysLowered );
        $this->setProperty( 'alwaysRaised', $this->alwaysRaised );
        $this->setProperty( 'modal', $this->modal );
        $this->setProperty( 'centerscreen', $this->centerscreen );
        $this->setProperty( 'dependent', $this->dependent );
        $this->setProperty( 'dialog', $this->dialog );
        $this->setProperty( 'resizable', $this->resizable );
        $this->setProperty( 'hostWindow', $this->hostWindow );
        $this->setProperty( 'parentHostWindow', $this->parent->hostWindow );
        $this->callMethod( 'open' );

        $this->parent->callMethod( 'addChildSilent', array($this) );

        $this->setEventHandler( 'initialized', $this, 'initWindow' );
    }

    public function detach() {
        $this->parent->callMethod( 'removeChildSilent', array($this) );
        $this->unRegister();
    }

    public function close() {
        $this->callMethod( 'close' );
        $this->closeWindow();
    }

    public function initWindow($event) {
        $this->title( $this->title );
        $this->orient('vertical');
        $this->setEventHandler( 'hasClosed', $this, 'closeWindow' );
        $this->focus();
        $this->init();
    }

    public function init() {
        if (isset($this->initChildren)) {
            $this->addChild( $this->initChildren );
        }

        $this->sizeToContent();

        if ($this->dependent && $this->centerscreen) {
            $this->callMethod( 'windowObject.moveToAlertPosition' );
        } elseif ($this->centerscreen) {
            $this->callMethod( 'windowObject.centerWindowOnScreen' );
        }
    }

    public function closeWindow() {
        $this->hasClosed = TRUE;
        $this->preventClose = FALSE;
        $this->parent->hostWindow->focus();
        $this->parent->removeChild( $this );

    }

    public function title($title = NULL) {
        if ($title === NULL) {
            return $this->title;
        }

        $this->title = $title;

        if ($this->isRegistered) {
            $this->callMethod( 'setTitle', array($title) );
        }

        return $this;
    }
}


/**
 * Dialog element
 *
 * @link https://developer.mozilla.org/en/XUL/dialog
 * @package API
 * @subpackage Windows
*/

class XULDialog extends XULChildWindow
{
    public $alwaysLowered   = FALSE;
    public $alwaysRaised    = FALSE;
    public $modal       = FALSE;
    public $centerscreen    = TRUE;
    public $dependent   = TRUE;
    public $dialog      = TRUE;
    public $resizable   = FALSE;
    public $title       = 'SiteFusion Dialog';

    public $buttons     = 'accept,cancel';
    public $remoteConstructor = 'Dialog';
    public $initAttributes = array( 'buttonlabelaccept', 'buttonlabelcancel', 'buttonlabelhelp', 'buttonlabeldisclosure', 'buttonlabelextra1', 'buttonlabelextra2' );

    public function initWindow($event) {
        $this->title( $this->title );
        $this->orient('vertical');
        $this->setProperty( 'element.buttons', $this->buttons );
        $this->setEventHandler( 'hasClosed', $this, 'closeWindow' );
        $this->init();
    }

    public function buttonlabelaccept($label = NULL) {
        if ($label === NULL) {
            return $this->buttonlabelaccept;
        }

        $this->buttonlabelaccept = $label;

        if ($this->isRegistered) {
            $this->setProperty( 'element.getButton("accept").label', $label );
        }

        return $this;
    }

    public function buttonlabelcancel($label = NULL) {
        if ($label === NULL) {
            return $this->buttonlabelcancel;
        }

        $this->buttonlabelcancel = $label;

        if ($this->isRegistered) {
            $this->setProperty( 'element.getButton("cancel").label', $label );
        }

        return $this;
    }

    public function buttonlabelhelp($label = NULL) {
        if ($label === NULL) {
            return $this->buttonlabelhelp;
        }

        $this->buttonlabelhelp = $label;

        if ($this->isRegistered) {
            $this->setProperty( 'element.getButton("help").label', $label );
        }

        return $this;
    }

    public function buttonlabeldisclosure($label = NULL) {
        if ($label === NULL) {
            return $this->buttonlabeldisclosure;
        }

        $this->buttonlabeldisclosure = $label;

        if ($this->isRegistered) {
            $this->setProperty( 'element.getButton("disclosure").label', $label );
        }

        return $this;
    }

    public function buttonlabelextra1($label = NULL) {
        if ($label === NULL) {
            return $this->buttonlabelextra1;
        }

        $this->buttonlabelextra1 = $label;

        if ($this->isRegistered) {
            $this->setProperty( 'element.getButton("extra1").label', $label );
        }

        return $this;
    }

    public function buttonlabelextra2($label = NULL) {
        if ($label === NULL) {
            return $this->buttonlabelextra2;
        }

        $this->buttonlabelextra2 = $label;

        if ($this->isRegistered) {
            $this->setProperty( 'element.getButton("extra2").label', $label );
        }

        return $this;
    }

    public function disableButton($button = 'accept', $disabled = TRUE) {
        if ($disabled) {
            $this->callMethod("element.getButton('".$button."').setAttribute", array('disabled' ,'true'));
        } else {
            $this->callMethod("element.getButton('".$button."').removeAttribute", array('disabled'));
        }
    }
}


/**
 * PrefWindow element
 *
 * @link https://developer.mozilla.org/en/XUL/prefwindow
 * @package API
 * @subpackage Windows
*/

class XULPrefWindow extends XULDialog {

    public $remoteConstructor = 'PrefWindow';

    /**
     * Makes the given pane active in the prefwindow
     *
     * @param XULPrefPane   $pane   Pane to activate
     */
    public function showPane($pane) {
        if (!$this->isRegistered) {
            throw new SFException( "Can't call showPane() on unregistered PrefWindow" );
        }

        $this->callMethod( 'showPane', array($pane) );
    }
}


/**
 * PrefPane element
 *
 * @link https://developer.mozilla.org/en/XUL/prefpane
 * @package API
 * @subpackage Windows
*/
class XULPrefPane extends BranchNode {
    public $remoteConstructor = 'PrefPane';
    public $initAttributes = array( 'helpURI', 'label', 'image' );

    /**
     * Dynamic Constructor
     *
     * @param string    $label
     * @param string    $image
     * @param mixed     $childNodes
    */
    public function __construct() {
        if (func_num_args()) {
            $args = func_get_args();

            if (count($args) && is_string($args[0])) {
                $this->label( array_shift($args) );
            }
            if (count($args) && is_string($args[0])) {
                $this->image( array_shift($args) );
            }

            parent::__construct( $args );
        } else {
            parent::__construct();
        }
    }

    public function helpURI($href = NULL) {
        return $this->attributeMethod( 'helpURI', (string) $href );
    }

    public function image($src = NULL) {
        if ($src === NULL) {
            return (isset($this->image) ? $this->image : NULL);
        }

        $this->image = $src;

        if ($this->isRegistered) {
            $this->callMethod( 'image', $this->image );
        }

        return $this;
    }
}


/**
 * OS-native prompt service object
 *
 * @property bool   $result     Whether the dialog was accepted by the user (e.g. clicked OK)
 * @property bool   $checkState State of the checkbox (if any)
 * @property mixed  $value      Value of the textbox or selected item in the list
 * @property string $username   Supplied username
 * @property string $password   Supplied password
 * @package API
 * @subpackage Windows
 */
class PromptService extends Node {

    const BUTTON_POS_0 = 1;
    const BUTTON_POS_1 = 256;
    const BUTTON_POS_2 = 65536;

    const BUTTON_TITLE_OK = 1;
    const BUTTON_TITLE_CANCEL = 2;
    const BUTTON_TITLE_YES = 3;
    const BUTTON_TITLE_NO = 4;
    const BUTTON_TITLE_SAVE = 5;
    const BUTTON_TITLE_DONT_SAVE = 6;
    const BUTTON_TITLE_REVERT = 7;
    const BUTTON_TITLE_IS_STRING = 127;

    const BUTTON_POS_0_DEFAULT = 0;
    const BUTTON_POS_1_DEFAULT = 16777216;
    const BUTTON_POS_2_DEFAULT = 33554432;

    const BUTTON_DELAY_ENABLE = 67108864;
    const STD_OK_CANCEL_BUTTONS = 513;
    const STD_YES_NO_BUTTONS = 1027;

    public $remoteConstructor = 'PromptService';

    public $promptInProgress = FALSE;
    private $handlerObj = NULL;
    private $handlerMethod = NULL;
    private $carryValue = NULL;
    private $closure = NULL;

    public function attach() {
        parent::attach();
        $this->setEvent( 'yield', MSG_SEND, $this, 'yieldCollect' );
    }

    /**
     * Shows a alert box with an OK button
     *
     * @param string    $title          Title of the dialog box
     * @param string    $text           Body text
     */
    public function alert($title, $text) {
        $this->callMethod( 'alert', array($title,$text) );
    }

    /**
     * Shows an alert box with an OK button and a checkbox
     *
     * @param string    $title          Title of the dialog box
     * @param string    $text           Body text
     * @param string    $checkMsg       Label text of the checkbox
     * @param bool      $checkState     Initial state of the checkbox
     * @param object    $handlerObj     Object to call the handler method on
     * @param string    $handlerMethod  Method to call on the handler object
     * @param mixed     $carryValue     Arbitrary value to pass on to the handler function
     */
    public function alertCheck($title, $text, $checkMsg, $checkState, $handlerObj, $handlerMethod, $carryValue = NULL) {
        if ($this->promptInProgress) {
            throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
        }

        $this->promptInProgress = TRUE;
        $this->handlerObj = $handlerObj;
        $this->handlerMethod = $handlerMethod;
        $this->carryValue = $carryValue;
        $this->callMethod( 'alertCheck', array($title,$text,$checkMsg,$checkState) );
    }

    /**
     * Shows a confirm box with OK and Cancel buttons
     *
     * @param string    $title          Title of the dialog box
     * @param string    $text           Body text
     * @param object    $handlerObj     Object to call the handler method on
     * @param string    $handlerMethod  Method to call on the handler object
     * @param mixed     $carryValue     Arbitrary value to pass on to the handler function
     */
    public function confirm($title, $text, $handlerObj, $handlerMethod, $carryValue = NULL) {
        if ($this->promptInProgress) {
            throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
        }

        $this->promptInProgress = TRUE;
        $this->handlerObj = $handlerObj;
        $this->handlerMethod = $handlerMethod;
        $this->carryValue = $carryValue;
        $this->callMethod( 'confirm', array($title,$text) );
    }

    /**
     * Shows a confirm box with OK and Cancel buttons
     *
     * @param string    $title          Title of the dialog box
     * @param string    $text           Body text
     * @param object    $closure        Closure
     */
    public function confirmClosure($title, $text, $closure) {
        if ($this->promptInProgress) {
            throw new SFException('PromptService: a prompt is already in progress', ERR_REPORT_APP);
        }

        $this->promptInProgress = TRUE;
        $this->closure = $closure;
        $this->callMethod('confirm', array($title, $text));
    }

    /**
     * Shows a confirm box with OK and Cancel buttons and a checkbox
     *
     * @param string    $title          Title of the dialog box
     * @param string    $text           Body text
     * @param string    $checkMsg       Label text of the checkbox
     * @param bool      $checkState     Initial state of the checkbox
     * @param object    $handlerObj     Object to call the handler method on
     * @param string    $handlerMethod  Method to call on the handler object
     * @param mixed     $carryValue     Arbitrary value to pass on to the handler function
     */
    public function confirmCheck($title, $text, $checkMsg, $checkState, $handlerObj, $handlerMethod, $carryValue = NULL) {
        if ($this->promptInProgress) {
            throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
        }

        $this->promptInProgress = TRUE;
        $this->handlerObj = $handlerObj;
        $this->handlerMethod = $handlerMethod;
        $this->carryValue = $carryValue;
        $this->callMethod( 'confirmCheck', array($title,$text,$checkMsg,$checkState) );
    }

    /**
     * Shows a custom confirm box with up to three buttons and a checkbox
     *
     * @param string    $title          Title of the dialog box
     * @param string    $text           Body text
     * @param int       $buttonFlags    Button flags
     * @param string    $button0Title   Optional custom title for button 0
     * @param string    $button1Title   Optional custom title for button 1
     * @param string    $button2Title   Optional custom title for button 2
     * @param string    $checkMsg       Label text of the checkbox
     * @param bool      $checkState     Initial state of the checkbox
     * @param object    $handlerObj     Object to call the handler method on
     * @param string    $handlerMethod  Method to call on the handler object
     * @param mixed     $carryValue     Arbitrary value to pass on to the handler function
     *
     * @link https://developer.mozilla.org/en/nsIPromptService#confirmEx example
     */
    public function confirmEx($title, $text, $buttonFlags, $button0Title, $button1Title, $button2Title, $checkMsg, $checkState, $handlerObj, $handlerMethod, $carryValue = NULL) {
        if ($this->promptInProgress) {
            throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
        }

        $this->promptInProgress = TRUE;
        $this->handlerObj = $handlerObj;
        $this->handlerMethod = $handlerMethod;
        $this->carryValue = $carryValue;
        $this->callMethod( 'confirmEx', array($title,$text,$buttonFlags,$button0Title,$button1Title,$button2Title,$checkMsg,$checkState) );
    }

    /**
     * Shows a custom confirm box with up to three buttons and a checkbox
     *
     * @param string    $title          Title of the dialog box
     * @param string    $text           Body text
     * @param int       $buttonFlags    Button flags
     * @param string    $button0Title   Optional custom title for button 0
     * @param string    $button1Title   Optional custom title for button 1
     * @param string    $button2Title   Optional custom title for button 2
     * @param string    $checkMsg       Label text of the checkbox
     * @param bool      $checkState     Initial state of the checkbox
     * @param object    $closure        Closure
     *
     * @link https://developer.mozilla.org/en/nsIPromptService#confirmEx example
     */
    public function confirmExClosure($title, $text, $buttonFlags, $button0Title, $button1Title, $button2Title, $checkMsg, $checkState, $closure) {
        if ($this->promptInProgress) {
            throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
        }

        $this->promptInProgress = TRUE;
        $this->closure = $closure;
        $this->callMethod( 'confirmEx', array($title,$text,$buttonFlags,$button0Title,$button1Title,$button2Title,$checkMsg,$checkState) );
    }

    /**
     * Shows a prompt box with a textinput and OK and Cancel buttons
     *
     * @param string    $title          Title of the dialog box
     * @param string    $text           Body text
     * @param string    $textValue      Initial value of the textinput
     * @param object    $handlerObj     Object to call the handler method on
     * @param string    $handlerMethod  Method to call on the handler object
     * @param mixed     $carryValue     Arbitrary value to pass on to the handler function
     */
    public function prompt($title, $text, $textValue, $handlerObj, $handlerMethod, $carryValue = NULL) {
        if ($this->promptInProgress) {
            throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
        }

        $this->promptInProgress = TRUE;
        $this->handlerObj = $handlerObj;
        $this->handlerMethod = $handlerMethod;
        $this->carryValue = $carryValue;
        $this->callMethod( 'prompt', array($title,$text,$textValue,NULL,NULL) );
    }

    /**
     * Shows a prompt box with a textinput and OK and Cancel buttons
     *
     * @param string    $title          Title of the dialog box
     * @param string    $text           Body text
     * @param string    $textValue      Initial value of the textinput
     * @param object    $closure        Closure
     */
    public function promptClosure($title, $text, $textValue, $closure) {
        if ($this->promptInProgress) {
            throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
        }

        $this->promptInProgress = TRUE;
        $this->closure = $closure;
        $this->callMethod( 'prompt', array($title,$text,$textValue,NULL,NULL) );
    }

    /**
     * Shows a prompt box with a textinput, a checkbox and OK and Cancel buttons
     *
     * @param string    $title          Title of the dialog box
     * @param string    $text           Body text
     * @param string    $textValue      Initial value of the textinput
     * @param string    $checkMsg       Label text of the checkbox
     * @param bool      $checkState     Initial state of the checkbox
     * @param object    $handlerObj     Object to call the handler method on
     * @param string    $handlerMethod  Method to call on the handler object
     * @param mixed     $carryValue     Arbitrary value to pass on to the handler function
     */
    public function promptCheck($title, $text, $textValue, $checkMsg, $checkState, $handlerObj, $handlerMethod, $carryValue = NULL) {
        if ($this->promptInProgress) {
            throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
        }

        $this->promptInProgress = TRUE;
        $this->handlerObj = $handlerObj;
        $this->handlerMethod = $handlerMethod;
        $this->carryValue = $carryValue;
        $this->callMethod( 'prompt', array($title,$text,$textValue,$checkMsg,$checkState) );
    }

    /**
     * Shows a prompt box with username and password textinputs and OK and Cancel buttons
     *
     * @param string    $title          Title of the dialog box
     * @param string    $text           Body text
     * @param string    $username       Initial value of the username textinput
     * @param string    $password       Initial value of the password textinput
     * @param object    $handlerObj     Object to call the handler method on
     * @param string    $handlerMethod  Method to call on the handler object
     * @param mixed     $carryValue     Arbitrary value to pass on to the handler function
     */
    public function promptUsernameAndPassword($title, $text, $username, $password, $handlerObj, $handlerMethod, $carryValue = NULL) {
        if ($this->promptInProgress) {
            throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
        }

        $this->promptInProgress = TRUE;
        $this->handlerObj = $handlerObj;
        $this->handlerMethod = $handlerMethod;
        $this->carryValue = $carryValue;
        $this->callMethod( 'promptUsernameAndPassword', array($title,$text,$username,$password,NULL,NULL) );
    }

    /**
     * Shows a prompt box with username and password textinputs, a checkbox and OK and Cancel buttons
     *
     * @param string    $title          Title of the dialog box
     * @param string    $text           Body text
     * @param string    $username       Initial value of the username textinput
     * @param string    $password       Initial value of the password textinput
     * @param string    $checkMsg       Label text of the checkbox
     * @param bool      $checkState     Initial state of the checkbox
     * @param object    $handlerObj     Object to call the handler method on
     * @param string    $handlerMethod  Method to call on the handler object
     * @param mixed     $carryValue     Arbitrary value to pass on to the handler function
     */
    public function promptUsernameAndPasswordCheck($title, $text, $username, $password, $checkMsg, $checkState, $handlerObj, $handlerMethod, $carryValue = NULL) {
        if ($this->promptInProgress) {
            throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
        }

        $this->promptInProgress = TRUE;
        $this->handlerObj = $handlerObj;
        $this->handlerMethod = $handlerMethod;
        $this->carryValue = $carryValue;
        $this->callMethod( 'promptUsernameAndPassword', array($title,$text,$username,$password,$checkMsg,$checkState) );
    }

    /**
     * Shows a prompt box with a password textinput and OK and Cancel buttons
     *
     * @param string    $title          Title of the dialog box
     * @param string    $text           Body text
     * @param string    $password       Initial value of the password textinput
     * @param object    $handlerObj     Object to call the handler method on
     * @param string    $handlerMethod  Method to call on the handler object
     * @param mixed     $carryValue     Arbitrary value to pass on to the handler function
     */
    public function promptPassword($title, $text, $password, $handlerObj, $handlerMethod, $carryValue = NULL) {
        if ($this->promptInProgress) {
            throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
        }

        $this->promptInProgress = TRUE;
        $this->handlerObj = $handlerObj;
        $this->handlerMethod = $handlerMethod;
        $this->carryValue = $carryValue;
        $this->callMethod( 'promptPassword', array($title,$text,$password,NULL,NULL) );
    }

    /**
     * Shows a prompt box with a password textinput, a checkbox and OK and Cancel buttons
     *
     * @param string    $title          Title of the dialog box
     * @param string    $text           Body text
     * @param string    $password       Initial value of the password textinput
     * @param string    $checkMsg       Label text of the checkbox
     * @param bool      $checkState     Initial state of the checkbox
     * @param object    $handlerObj     Object to call the handler method on
     * @param string    $handlerMethod  Method to call on the handler object
     * @param mixed     $carryValue     Arbitrary value to pass on to the handler function
     */
    public function promptPasswordCheck($title, $text, $password, $checkMsg, $checkState, $handlerObj, $handlerMethod, $carryValue = NULL) {
        if ($this->promptInProgress) {
            throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
        }

        $this->promptInProgress = TRUE;
        $this->handlerObj = $handlerObj;
        $this->handlerMethod = $handlerMethod;
        $this->carryValue = $carryValue;
        $this->callMethod( 'promptPassword', array($title,$text,$password,$checkMsg,$checkState) );
    }

    /**
     * Shows a prompt box with a dropdown box and OK and Cancel buttons
     *
     * @param string    $title          Title of the dialog box
     * @param string    $text           Body text
     * @param array     $list           Array of options (strings)
     * @param int       $selectedIndex  Initial index to select (broken)
     * @param object    $handlerObj     Object to call the handler method on
     * @param string    $handlerMethod  Method to call on the handler object
     * @param mixed     $carryValue     Arbitrary value to pass on to the handler function
     */
    public function select($title, $text, $list, $selectedIndex, $handlerObj, $handlerMethod, $carryValue = NULL) {
        if ($this->promptInProgress) {
            throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
        }

        $this->promptInProgress = TRUE;
        $this->handlerObj = $handlerObj;
        $this->handlerMethod = $handlerMethod;
        $this->carryValue = $carryValue;
        $this->callMethod( 'select', array($title, $text, $list, (int) $selectedIndex) );
    }

    /**
     * [INTERNAL FUNCTION] Collects the results of a prompt
    */
    public function yieldCollect($e, $result, $checkState, $value, $username, $password) {
        $this->result = $result;
        $this->checkState = $checkState;
        $this->value = $value;
        $this->username = $username;
        $this->password = $password;

        $this->promptInProgress = FALSE;
        if ($this->closure) {
            $this->closure->__invoke($this);
            $this->closure = NULL;
        } else {
            $ho = $this->handlerObj;
            $hm = $this->handlerMethod;
            $cv = $this->carryValue;

            $this->handlerObj = NULL;
            $this->handlerMethod = NULL;
            $this->carryValue = NULL;

            if ($ho !== NULL && $hm !== NULL) {
                $ho->{$hm}($this, $cv);
            }
        }
    }
}

/**
 * OS-native alert notification node
 *
 * This node displays a notification box on the client
 * Alert notifications can be made clickable, making them fire 'command' events when clicked. They also fire a 'finished'
 * event when disappearing and a 'focus' event when it is first shown.
 * The notification appears as soon as it's registered (added to a registered parent node).
 * @package API
 * @subpackage Windows
 */
class AlertNotification extends Node {

    const ORIGIN_BOTTOMRIGHT_SLIDEVERTICAL   = 0;
    const ORIGIN_BOTTOMRIGHT_SLIDEHORIZONTAL = 1;
    const ORIGIN_BOTTOMLEFT_SLIDEVERTICAL    = 2;
    const ORIGIN_BOTTOMLEFT_SLIDEHORIZONTA   = 3;
    const ORIGIN_TOPRIGHT_SLIDEVERTICAL      = 4;
    const ORIGIN_TOPRIGHT_SLIDEHORIZONTAL    = 5;
    const ORIGIN_TOPLEFT_SLIDEVERTICAL       = 6;
    const ORIGIN_TOPLEFT_SLIDEHORIZONTA      = 7;

    public $remoteConstructor = 'AlertNotification';

    /**
     * If the notification should send an event when clicked
     * @var boolean
     */
    public $textClickable = FALSE;

    /**
     * The origin of the alert, this should be one of the AlertNotification::ORIGIN_ constants, defaults to AlertNotification::ORIGIN_TOPRIGHT_SLIDEHORIZONTAL
     * @var integer
     */
    public $alertOrigin = NULL;

    /**
     * The url to the image that should be shown
     * @var string
     */
    public $image;

    /**
     * The title to show at the top of the notification
     * @var string
     */
    public $title;

    /**
     * The text to show in the notification
     * @var string
     */
    public $text;

    /**
     * The name at which this notification is known, if NULL it defaults to SiteFusion
     * @var string
     */
    public $name = NULL;

    /**
     * Contructs a new AlertNotification
     * @param string  $image         Path to the image to display (empty string to display no image)
     * @param string  $title         Title of the alert
     * @param string  $text          Text of the alert
     * @param string  $name          Name of the notification type
     * @param bool    $textClickable Indicates whether the notification should be clickable. If TRUE, the notification fires 'command' event when clicked.
     * @param integer $alertOrigin   The origin of the alert, defaults to AlertNotification::ORIGIN_TOPRIGHT_SLIDEHORIZONTAL
     */
    public function __construct($image, $title, $text, $name = NULL, $textClickable = FALSE, $alertOrigin = NULL) {
        $this->image = $image;
        $this->title = $title;
        $this->text = $text;
        $this->name = $name;
        $this->textClickable = $textClickable;
        $this->alertOrigin = ($alertOrigin === NULL ? self::ORIGIN_TOPRIGHT_SLIDEHORIZONTAL : $alertOrigin);
    }

    /**
     * The attach function
     * @access private
     */
    public function attach() {
        parent::attach();
        $this->setEvent( 'finished', MSG_SEND, $this, 'onFinished' );

        $this->callMethod( 'showAlertNotification', array( $this->image, $this->title, $this->text, $this->name, $this->textClickable, $this->alertOrigin ) );
    }

    /**
     * Internal handler for the 'finished' event, removes the node
     * @access private
     */
    public function onFinished($e) {
        $this->removeNode();
    }
}
