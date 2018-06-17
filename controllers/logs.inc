<?php
require_once  'default.inc';
class LogsController extends DefaultController {

	public function model(){
		$model = parent::model();
		return $model;
	}

	public function indexAction(){
		$this->logAction();
	}

	public function logdetailsAction(){
		$db = DataAccess::instance();
		$logaction = $db->readLogAction($this->getParam('logaction'));
		$context = $db->getContext($logaction['context']);
		$canWrite =  ACL::allowed($this->_userid(), ACL::P_WRITE, ACL::CONTEXT, $context['id']);
		if(!$canWrite){
			$this->_warning('This context is read-only');
		}
		$model = $this->model();
		$model['title'] =  $context['name'] . ': Log action: #' . $this->getParam('logaction');
		$model['page'] = 'logaction';
		$model['data'] = array(
			'context' => $context,
			'logaction' => $logaction,
			'logchanges' => $db->readLogChanges($this->getParam('logaction')),
			'canRevert' => $canWrite
		);
		$v = new MVC_Viewer($model);
		$v->render('layout');
	}
	
	public function revertAction(){
		$db = DataAccess::instance();
		$logaction = $db->readLogAction($this->getParam('logaction'));
		$context = $db->getContext($logaction['context']);
		if(ACL::allowed($this->_userid(), ACL::P_WRITE, ACL::CONTEXT, $logaction['context'])){
			$model = $this->model();
			$db->revertChange($logaction['context'], $logaction['id'], $model['userid']);
			redirect(route('contexts', 'context', array('context' => $logaction['context'])));
		}else{
			$this->_danger('You are not allowed to do it.');
			$this->logdetailsAction();
		}
	}
}
