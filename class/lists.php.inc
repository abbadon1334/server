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
 * @subpackage Lists
*/



/**
 * Listbox element
 *
 * @link https://developer.mozilla.org/en/XUL/listbox
 * @package API
 * @subpackage Lists
*/

class XULListBox extends BranchNode
{
    public $remoteConstructor = 'ListBox';
    public $initAttributes = array( 'seltype', 'rows' );
    public $selectedItems = array();
    public $selectedItem = NULL;
    public $selectedValue = NULL;


    /**
     * Dynamic Constructor
     *
     * @param int       $flex
     * @param string    $seltype    String indicating selection style 'single' or 'multiple'
     * @param mixed     $childNodes
    */

    public function __construct() {
        if( func_num_args() ) {
            $args = func_get_args();

            if( count($args) && is_int($args[0]) )
                $this->flex( array_shift($args) );
            if( count($args) && is_string($args[0]) )
                $this->seltype( array_shift($args) );

            parent::__construct( $args );
        }
        else parent::__construct();

        $this->setEventHandler( 'yield', $this, 'yieldCollect' );
    }

    public function attach() {
        parent::attach();

        if( count($this->selectedItems) )
            $this->selectItem( $this->selectedItems );
    }

    public function selectItem($items) {
        $items = (is_array($items) ? $items : array($items));
        $this->selectedItems = $items;

        if ($this->isRegistered) {
            $this->callMethod('selectItem', array($items));
        }
    }

    public function clearSelection() {
        $this->selectedItems = array();
        $this->selectedItem = NULL;
        $this->selectedValue = NULL;
        $this->callMethod('clearSelection');
    }

    public function removeItems()
    {
        for ($i=0; $i < count($this->children); $i++)
        {
            if ($this->children[$i] instanceof XULListItem)
            {
                $this->removeChild($this->children[$i]);
                $i--;
            }

        }
    }

    public function yieldCollect() {
        $args = func_get_args();
        $e = array_shift( $args );

        $this->selectedItem = array_shift( $args );
        $this->selectedValue = (isset($this->selectedItem->value) ? $this->selectedItem->value : NULL);
        $this->selectedItems = $args;
    }

    public function seltype( $type = NULL ) {
        return $this->attributeMethod( 'seltype', ($type === NULL ? NULL : ($type == 'single' ? 'single':'multiple')) );
    }

    public function rows( $amount = NULL ) {
        return $this->attributeMethod( 'rows', $amount );
    }
}


/**
 * Listitem element
 *
 * @link https://developer.mozilla.org/en/XUL/listitem
 * @package API
 * @subpackage Lists
*/

class XULListItem extends BranchNode
{
    public $remoteConstructor = 'ListItem';
    public $initAttributes = array( 'image' );


    /**
     * Dynamic Constructor
     *
     * @param string    $label
     * @param string    $value
     * @param string    $image
     * @param mixed     $childNodes
    */

    public function __construct() {
        if( func_num_args() ) {
            $args = func_get_args();

            if( count($args) && is_string($args[0]) )
                $this->label( array_shift($args) );
            if( count($args) && is_string($args[0]) )
                $this->value( array_shift($args) );
            if( count($args) && is_string($args[0]) )
                $this->image( array_shift($args) );

            parent::__construct( $args );
        }
        else parent::__construct();
    }

    public function image( $src = NULL ) {
        if( $src === NULL )
            return (isset($this->image) ? $this->image : NULL);

        $this->image = $src;

        if( $this->isRegistered ) {
            $this->callMethod( 'image', $this->image );
            $this->setAttribute( 'class', 'listitem-iconic' );
        }

        return $this;
    }
}


/**
 * Listcols element
 *
 * @link https://developer.mozilla.org/en/XUL/listcols
 * @package API
 * @subpackage Lists
*/

class XULListCols extends BranchNode
{
    public $remoteConstructor = 'ListCols';
}


/**
 * Listcol element
 *
 * @link https://developer.mozilla.org/en/XUL/listcol
 * @package API
 * @subpackage Lists
*/

class XULListCol extends BranchNode
{
    public $remoteConstructor = 'ListCol';


    /**
     * Dynamic Constructor
     *
     * @param int       $flex
     * @param mixed     $childNodes
    */

    public function __construct() {
        if( func_num_args() ) {
            $args = func_get_args();

            if( count($args) && is_integer($args[0]) )
                $this->flex( array_shift($args) );

            parent::__construct( $args );
        }
        else parent::__construct();
    }
}


/**
 * Listhead element
 *
 * @package API
 * @subpackage Lists
 * @link https://developer.mozilla.org/en/XUL/listhead
*/

class XULListHead extends BranchNode
{
    public $remoteConstructor = 'ListHead';
}


/**
 * Listheader element
 *
 * @link https://developer.mozilla.org/en/XUL/listheader
*/

class XULListHeader extends BranchNode
{
    public $remoteConstructor = 'ListHeader';


    /**
     * Dynamic Constructor
     *
     * @param string    $label
     * @param mixed     $childNodes
    */

    public function __construct() {
        if( func_num_args() ) {
            $args = func_get_args();

            if( count($args) && is_string($args[0]) )
                $this->label( array_shift($args) );

            parent::__construct( $args );
        }
        else parent::__construct();
    }
}


/**
 * Listcell element
 *
 * @link https://developer.mozilla.org/en/XUL/listcell
*/

class XULListCell extends BranchNode
{
    public $remoteConstructor = 'ListCell';


    /**
     * Dynamic Constructor
     *
     * @param string    $label
     * @param mixed     $childNodes
    */

    public function __construct() {
        if( func_num_args() ) {
            $args = func_get_args();

            if( count($args) && is_string($args[0]) )
                $this->label( array_shift($args) );

            parent::__construct( $args );
        }
        else parent::__construct();
    }
}


/**
 * RichListBox element
 *
 * @link https://developer.mozilla.org/en/XUL/richlistbox
*/

class XULRichListBox extends XULListBox
{
    public $remoteConstructor = 'RichListBox';
}


/**
 * RichListItem element
 *
 * @link https://developer.mozilla.org/en/XUL/richlistitem
*/

class XULRichListItem extends XULListItem
{
    public $remoteConstructor = 'RichListItem';


    /**
     * Dynamic Constructor
     *
     * @param string    $value
     * @param mixed     $childNodes
    */

    public function __construct() {
        if( func_num_args() ) {
            $args = func_get_args();

            if( count($args) && is_string($args[0]) )
                $this->value( array_shift($args) );

            Node::__construct( $args );
        }
        else Node::__construct();
    }
}

