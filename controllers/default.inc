<?php
require_once dirname(__FILE__) . '/../dataaccess.inc';
require_once dirname(__FILE__) . '/../acl.inc';

class DefaultController extends MVC_Controller {
	protected $_model = NULL;
	public function __construct($args = array()){
		parent::__construct($args);
		
		// Setup model
		$this->_model = array(
				'title' => '',
				'nav' => array(
						'Items' => route('items'),
						'Contexts' => route('contexts'),
				),
				'page' => 'index',
				'data' => array()
		);
		
		if($this->_enableSession()){
			session_start();
			
			if(!isset($_SESSION) || !isset($_SESSION['login']) || $_SESSION['login'] != "1" || !isset($_SESSION['userid']) || $_SESSION['userid'] == 0){
				$_SESSION['redirectto'] = $_SERVER['REQUEST_URI'];
				$this->_parameters[$this->_action_parameter] = 'login';
				
			}

			$db = DataAccess::instance();
			if(isset($_SESSION['userid'])){
				// If user is not active, prevent from doing anything
				if(!ACL::member($_SESSION['userid'], ACL::R_ACTIVE)){
					$u = $db->user($_SESSION['userid']);
					$this->logoutAction();
					error_forbidden("User " . $u['name'] . " is not active");
					die;
				}
				$this->_model['userid'] = $_SESSION['userid'];
				$this->_model['user'] = $db->user($_SESSION['userid']);
			}
		}
		
	}
	
	protected function _enableSession(){
		return true;
	}
	
	protected function _userid(){
		return $_SESSION['userid'];
	}
	
	/**
	 * @deprecated
	 * @param unknown $object_type
	 * @param unknown $object_id
	 */
	protected function _canWrite($object_type, $object_id){
		if(!ACL::allowed($this->_userid(), ACL::P_WRITE, $object_type, $object_id)){
			$this->actionNotAllowed();
		}
	}
	
	protected function actionNotAllowed(){
		//error_forbidden("Sorry, you are not authorized to perform this action.");
		$this->_forbidden();
	}
	
	public function model(){
		return $this->_model;
	}

	protected function _forbidden($reason = 'Forbidden'){
		header('HTTP/1.0 403 Forbidden', true, 403);
		$m = $this->model();
		$m['title'] = '';
		$m['nav'] = array();
		$m['actions'] = array();
		$m['data']['user'] = $m['user'];
		$m['data']['reason'] = $reason;
		$m['page'] = 'sorry';
		$v = new MVC_Viewer($m);
		$v->render('layout');
		die;
	}
	
	protected function _notfound($reason = 'Not found'){
		header('HTTP/1.0 404 Not found', true, 404);
		$m = $this->model();
		$m['title'] = '';
		$m['nav'] = array();
		$m['actions'] = array();
		$m['data']['user'] = $m['user'];
		$m['data']['reason'] = $reason;
		$m['page'] = 'sorry';
		$v = new MVC_Viewer($m);
		$v->render('layout');
		die;
	}
	
	protected function _malformed($reason = 'Malformed request'){
		header('HTTP/1.0 400 Bad request', true, 400);
		$m = $this->model();
		$m['title'] = '';
		$m['nav'] = array();
		$m['actions'] = array();
		$m['data']['user'] = $m['user'];
		$m['data']['reason'] = $reason;
		$m['page'] = 'sorry';
		$v = new MVC_Viewer($m);
		$v->render('layout');
		die;
	}
	
	public function indexAction(){
		$m = $this->model();
		$m['data']['user'] = $m['user'];
		$v = new MVC_Viewer($m);
		$v->render('layout');
	}

	public function loginAction(){
		$model = $this->model();
		$model['page'] = 'login';
		$model['hidenav'] = true;
		$model['title'] = 'contento';
		if ($_SERVER['REQUEST_METHOD'] === 'POST'){
			$ret = $this->validate(array('username' => array('NotNull', 'NotEmpty'),'password' => array('NotNull', 'NotEmpty')));
			if(count($ret) == 0){
				$db = DataAccess::instance();
				$userid = $db->login($this->getParam('username'), $this->getParam('password'));
				if($userid !== false){
					$_SESSION['login'] = 1;
					$_SESSION['userid'] = $userid;
					$rto = $_SESSION['redirectto'];
					unset($_SESSION['redirectto']);
					redirect($rto, 303);
				}
			}
		}
	
		$v = new MVC_Viewer($model);
		$v->render('login-layout');
	}
	

	public function logoutAction(){
		unset($_SESSION['user']);
		session_regenerate_id(FALSE);
		session_unset();
		header('Location: ' . route('default'));
	}
	
	
	
	protected function _message($type, $text){
		if(!isset($_SESSION['messages']) || !is_array($_SESSION['messages'])){
			$_SESSION['messages'] = array();
		}
		if(!isset($_SESSION['messages'][$type]) || !is_array($_SESSION['messages'][$type])){
			$_SESSION['messages'][$type] = array();
		}
		array_push($_SESSION['messages'][$type], $text);
	}

	protected function _info($t){
		$this->_message('info', $t); 
	}
	protected function _warning($t){
		$this->_message('warning', $t); 
	}
	protected function _danger($t){
		$this->_message('danger', $t); 
	}
	protected function _success($t){
		$this->_message('success', $t); 
	}
}