<?php
require_once  'default.inc';
class ItemsController extends DefaultController {

	public function model(){
		$model = parent::model();
		$model['title'] = 'Items';
		$typeId = $this->get('type', NULL);
		if($typeId !== NULL){
			$canWrite = ACL::allowed($this->_userid(), ACL::P_WRITE, ACL::TYPE, $typeId);
			$model['actions'] = array(
				'List items' => array('href' => route('items', 'list', array('type' => $typeId)), 'icon' => 'list', 'type' => 'info'),
				'Add items' =>  array('href' => ($canWrite) ? route('items', 'additems', array('type' => $typeId)):'', 'icon' => 'plus', 'type' => 'warning') ,
				'Delete' => array('href' =>  ($canWrite) ? route('items', 'deletetype', array('type' => $typeId)) : '', 'icon' => 'trash', 'type' => 'danger')
			);
			if(!$canWrite){
				$this->_warning('This collection is read-only');
			}
		}
		$model['page'] = 'items';
		return $model;
	}

	public function indexAction(){
		// Forward to typesAction
		$this->typesAction();
	}

	public function additemsAction(){
		$model = $this->model();
		$model['title'] = 'Add items';
		$model['page'] = 'additems';
		$model['actions_select'] = 'Add items';
		$db = DataAccess::instance();
		$type=NULL;
		if($this->has('type')){
			$type = array(
				'id'=>$this->get('type'), 
				'name'=>$db->getTypeName($this->get('type')));
		}
		$model['data'] = array(
				'types' => $db->types(),
				'type' => $type);
		$v = new MVC_Viewer($model);
		$v->render('layout');
	}

	public function newtypeAction(){
		$model = $this->model();
		$model['title'] = 'New type';
		$model['page'] = 'newtype';
		$v = new MVC_Viewer($model);
		$v->render('layout');
	}

	public function typesAction(){
		$model = $this->model();
		$model['title'] = 'Item collections';
		$db = DataAccess::instance();
		$types = $db->types($this->_userid());
		$model['page'] = 'types';
		$model['data'] = array('types' => $types);
		$v = new MVC_Viewer($model);
		$v->render('layout');
	}
	
	public function listAction(){
		$errs = $this->validate(array('type' => array('NotEmpty')));
		if(!empty($errs)){
			error_malformed($errs);
		}
		$model = $this->model();
		
		$db = DataAccess::instance();
		$typeId = $this->getParam('type');
		$items = $db->listItems($typeId);
		$typeName = $db->getTypeName($this->getParam('type'));
		$model['title'] = 'Collection #' . $typeId . ': "' . $typeName . '"';
		$model['page'] = 'listitems';
		$model['data'] = array('items' => $items, 'type' => $typeName);
		$model['actions_select'] = 'List items';
		$v = new MVC_Viewer($model);
		$v->render('layout');
	}

	public function deletetypeAction(){
		$typeId = $this->getParam('type'); // the typeId
		$db = DataAccess::instance();
		$items = $db->listItems($typeId);
		$typeName = $db->getTypeName($this->getParam('type'));
		$contexts = $db->listContexts($typeId);
		$model = $this->model();
		if(count($contexts)){ $this->_danger('This will also delete any associated context.');}
		$model['title'] = 'Delete collection #' . $typeId . ': "' . $typeName . '"';
		$model['page'] = 'deletetype';
		$model['actions_select'] = 'Delete type';
		$model['data'] = array('items' => $items, 'type' => $typeName, 'typeId' => $typeId, 'contexts' => $contexts);
		$v = new MVC_Viewer($model);
		$v->render('layout');
	}
	
	public function postdeletetypeAction(){
		$errs = $this->validate(array('delete' => array('NotEmpty')));
		if(!empty($errs)){
			error_malformed($errs);
		}
		$typeId = $this->getParam('delete'); // the typeId
		
		// ACL
		if(ACL::allowed($this->_userid(), ACL::P_WRITE, ACL::TYPE, $typeId)){
			$db = DataAccess::instance();
			$db->deleteType($typeId);
			$this->_success('Collection has been deleted.');
			header("Location: " . route('items', 'types'));
		}else{
			$this->_forbidden('You are not allowed to do this.');
		}
	}
	
	public function postitemsAction(){
		// Save items
		$typeId = $this->getParam('type'); // the typeId
		// ACL
		if(ACL::allowed($this->_userid(), ACL::P_WRITE, ACL::TYPE, $typeId)){
			$this->_additems($typeId);
			header("Location: " . route('items', 'list', array('type' => $typeId)));
		}else{
			$this->_forbidden('You are not allowed to do this.');
		}
	}

	private function _additems($typeId){
		$items = $this->getParam('items', '');
		$items = preg_split('/\n|\r/', $items, -1, PREG_SPLIT_NO_EMPTY);;
		if(count($items) > 0){
			$db = DataAccess::instance();
			$db->insertItems($typeId, $items);
			$this->_success(count($items) . ' items added.');
			// For each affected context
			$contexts = $db->listContexts($typeId);
			if(count($contexts) > 0){
				$updated = '';
				foreach($contexts as $c){
					if($updated != ''){
						$updated .= ', ';
					}
					$updated .= $c['name'];
				}
				$db->updateContextsRelations($typeId);
				$this->_success('Contexts updated: ' . $updated);
			}
		}
	}

	public function postnewtypeAction(){
		$errs = $this->validate(array('typeName' => array('NotEmpty')));
		if(count($errs)>0){
			error_malformed($errs);
		}
		
		// Save items
		$typeName = $this->getParam('typeName');
		$db = DataAccess::instance();
		$type = $db->insertType($typeName);
		
		// Grant ACL
		ACL::grantUser($this->_userid(), ACL::P_WRITE, 'type', $type);

		$this->_additems($type);
		$this->_success($typeName . ' collection created.');
		header("Location: " . route('items', 'list', array('type' => $type)));
	}
}