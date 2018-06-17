<?php
require_once  'default.inc';
class ContextsController extends DefaultController {

	public function model(){
		$model = parent::model();
		$model['title'] = 'Contexts';
		return $model;
	}

	public function indexAction(){
		$this->listAction();
	}

	public function logAction(){
		$db = DataAccess::instance();
		$model = $this->model();
		$contextId = $this->getParam('context');
		$canWrite = ACL::allowed($this->_userid(), ACL::P_WRITE, 'context', $contextId);
		if(!$canWrite){
			$this->_forbidden('Cannot edit context');
		}
		$context = $db->getContext($contextId);
		$model['title'] = 'Changes in context #' . $contextId . ': "' . $context['name'] . '"';
		$model['actions'] = $this->_contextActions($canWrite);
		$model['page'] = 'log';
		$model['data'] = array(
			'log' => $db->readLog()
		);

		$v = new MVC_Viewer($model);
		$v->render('layout');
	}

	public function listAction(){
		$db = DataAccess::instance();
		$model = $this->model();
		$list = $db->contexts($this->_userid());
		$model['page'] = 'listcontexts';
		$model['data'] = array('contexts' => $list);
		$v = new MVC_Viewer($model);
		$v->render('layout');
	}

	private function _cmp_by_name($a, $b) {
		return strcasecmp($a["name"], $b["name"]);
	}
	
	public function newAction(){
		$model = $this->model();
		$model['title'] = 'New Context';
		$model['page'] = 'newcontext';

		$db = DataAccess::instance();
		$list = $db->listTypes();
		usort($list, array($this, '_cmp_by_name'));
		$model['data'] = array('types' => $list, 'name' => $this->get('name',''));
		$v = new MVC_Viewer($model);
		$v->render('layout');
	}

	public function editAction(){
		$errors = $this->validate(array(
			'context' => array('NotNull','NotEmpty', 'Int')
		));
		if(count($errors)){
			$this->_malformed('Malformed request');
			return;
		}
		
		$db = DataAccess::instance();
		$contextId = $this->getParam('context');
		// ACL
		$canWrite = ACL::allowed( $this->_userid(), ACL::P_WRITE, 'context', $contextId );
		if(!$canWrite){
			$this->_forbidden('Cannot edit context');
			return;
		}
		
		$model = $this->model();
		$model['page'] = 'edit-context';
		$context = $db->getContext($contextId);
		//
		$model['title'] = 'Edit context #' . $contextId . ': "'. $context['name'].'"';
		$context['name'] = ($this->get('name', FALSE)) ? $this->get('name') : $context['name'];
		$context['description'] = ($this->get('description', FALSE)) ? $this->get('description') : $context['description'];
		$model['actions'] = $this->_contextActions($canWrite);
		$model['actions_select'] = 'Edit';
		
		$model['data'] = $context;
		
		$v = new MVC_Viewer($model);
		$v->render('layout');
	}
	
	
	public function postcontextAction(){
		if($_SERVER['REQUEST_METHOD'] === 'POST'){
			$db = DataAccess::instance();
			$oneOf = new MVC_OneOfVal($db->typeIds());
			$errors = $this->validate(array(
					'name' => array('NotNull','NotEmpty'),
					'objects' => array('NotNull','NotEmpty', 'Int', $oneOf),
					'attributes' => array('NotNull','NotEmpty', 'Int', $oneOf),
			));
			$this->sanitize(array(
					'objects' => array('intval'),
					'attributes' => array('intval'),
			));
			
			if(count($errors) === 0){
				$contextId = $db->getContextIdByName($this->get('name'));
				if($contextId){
					// A context with this name already exists!
					$this->_warning('A context with this name already exists. Please choose a different name.');
					$this->newAction();
					return;
				}
				$contextId = $db->createContext($this->getParam('name'), $this->getParam('description'), $this->getParam('objects'), $this->getParam('attributes'));

				// Grant WRITE access to created context
				ACL::grantUser($this->_userid(), ACL::P_WRITE, 'context', $contextId);
				
				$this->_success('Context ' . $this->getParam('name') . ' has been created.');
				redirect(route('contexts'));
			}else{
				$this->_malformed("Bad request");
			}
		}
	}
	
	public function posteditcontextAction(){
		if($_SERVER['REQUEST_METHOD'] === 'POST'){
			$db = DataAccess::instance();
			$errors = $this->validate(array(
				'name' => array('NotNull','NotEmpty'),
				'context' => array('NotNull','NotEmpty', 'Int')
			));
			
			if(count($errors) > 0){
				$this->_malformed("Bad request");
			}
			$contextId = $db->getContextIdByName ( $this->get ( 'name' ) );
			
			if ($contextId > 0 && $contextId != $this->get('context')) {
				// A context with this name already exists!
				$this->_warning ( 'A context with this name already exists. Please choose a different name.' );
				$this->editAction ();
				return;
			}
			$contextId = $db->saveContext ( $this->get ( 'context' ),  $this->get ( 'name' ), $this->get ( 'description' ) );
			$this->_success ( 'Context ' . $this->getParam ( 'name' ) . ' has been updated.' );
			redirect ( route ( 'contexts', 'relations', array ( 'context' => $this->get ( 'context' ) )));
		}
	}

	public function contextAction(){
		$this->relationsAction();
	}

	private function _contextActions($canWrite){
		return array(
				'Relations' => array('href' => route('contexts', 'relations', array('context' => $this->getParam('context'))), 'icon' => 'list', 'type' => 'primary'),
				'Log' => array( 'href' => ($canWrite) ? route('contexts', 'log', array('context' => $this->getParam('context'))) : '', 'icon' =>'list-alt', 'type' => 'info'),
				'Edit' => array( 'href' => ($canWrite) ? route('contexts', 'edit', array('context' => $this->getParam('context'))) : '', 'icon' =>'edit', 'type' => 'info'),
				'Create taxonomy' => array( 'href' => route('contexts', 'taxonomy', array('context' => $this->getParam('context'))), 'icon' => 'new-window', 'type' => 'success' ),
				'Export' => array( 'href' => route('contexts', 'doexport', array('context' => $this->getParam('context'))), 'icon' => 'export' , 'type' => 'primary'),
				'Delete' => array( 'href' => ($canWrite) ?  route('contexts', 'delete', array('context' => $this->getParam('context'))) : '', 'icon' => 'trash', 'type' => 'danger' )
		);
	}

	public function relationsAction(){
		$errors = $this->validate(array(
			'context' => array('NotNull','NotEmpty', 'Int')
		));
		if(count($errors)){
			$this->_malformed('Malformed request');
			return;
		}
		
		$db = DataAccess::instance();
		$model = $this->model();
		$model['page'] = 'relations';
		$contextId = $this->getParam('context');
		$context = $db->getContext($contextId);
		if(!$context){
			$this->_notfound();
			return;
		}
		$model['title'] = 'Relations of context #' . $contextId . ': "'. $context['name'].'"';
		
		// ACL
		$canWrite = ACL::allowed($this->_userid(), ACL::P_WRITE, 'context', $contextId);
		$model['actions'] = $this->_contextActions($canWrite);
		$model['actions_select'] = 'Relations';
		$opdone =  FALSE;
		// Perform operations if necessary
		if($this->getParam('performholds', NULL) !== NULL ){
			if(!$canWrite) $this->_forbidden('You are not allowed to do it.'); // ACL
			$oplogid = $db->holds($contextId, $this->getParam('performholds'), DataAccess::HOLDS, $model['userid']);
			$opdone = $this->getParam('performholds');
		}else if($this->getParam('performnotholds', NULL) !== NULL ){
			if(!$canWrite) $this->_forbidden('You are not allowed to do it.'); // ACL
			$oplogid = $db->holds($contextId, $this->getParam('performnotholds'), DataAccess::NOTHOLDS, $model['userid']);
			$opdone = $this->getParam('performnotholds');
		}else if($this->getParam('performunknown', NULL) !== NULL ){
			if(!$canWrite) $this->_forbidden('You are not allowed to do it.'); // ACL
			$oplogid = $db->unholds($contextId, $this->getParam('performunknown'), $model['userid']);
			$opdone = $this->getParam('performunknown');
		}

		if($opdone !== FALSE){
			$this->_success('Changed status of relation #' . $opdone . '. <a href="' . route('logs', 'logdetails', array('context'=> $contextId, 'logaction' => $oplogid)) . '">Undo</a>.');
		}
		
		// Prepare output
		$filterObject = NULL;
		if($this->getParam('filterObject','')!==''){
			$filterObject = $this->getParam('filterObject');
		}
		$filterAttribute = NULL;
		if($this->getParam('filterAttribute','')!==''){
			$filterAttribute = $this->getParam('filterAttribute');
		}
		$filterObjectMatches = NULL;
		if($this->getParam('filterObjectMatches','')!==''){
			$filterObjectMatches = $this->getParam('filterObjectMatches');
		}
		$filterAttributeMatches = NULL;
		if($this->getParam('filterAttributeMatches','')!==''){
			$filterAttributeMatches = $this->getParam('filterAttributeMatches');
		}
		$filterHolds = $this->getParam('filterHolds','');
		if($filterHolds !== ''){
			$this->sanitize(array('filterHolds' => array(new HoldsSan())));
		}else $filterHolds = NULL;
		$filterObjectHaving = $this->getParam('filterObjectHaving','');
		$foh = array();
		if($filterObjectHaving != ''){
			$foh = array($filterObjectHaving);
		}
		$filterAttributeHaving = $this->getParam('filterAttributeHaving','');
		$fah = array();
		if($filterAttributeHaving != ''){
			$fah = array($filterAttributeHaving);
		}

		// If there is the operations, perform it
		if($this->getParam('allholds', NULL) !== NULL){
			// TODO better do this from within a query?
			$vo = new HoldsSan();
			$hv = $vo->filter($this->getParam('allholds'));
			$relids = $this->getParam('relid', array());
			
			if(count($relids) > 0){
				if($hv !== -1){
					$logid = $db->holds($contextId, $relids, $hv, $model['userid']);
				}else{
					$logid = $db->unholds($contextId, $relids, $model['userid']);
				}
				$this->_success('Changed status of ' . count($relids) . ' relations. <a href="' . route('logs', 'logdetails', array('context'=> $contextId, 'logaction' => $logid)) . '">Undo</a>.');
			}
		}
		
		$stats = $db->contextStats($contextId);
		$relations = $db->relations($contextId, $filterObject, $filterAttribute, $this->getParam('filterHolds'), $foh, $fah, $filterObjectMatches, $filterAttributeMatches);
		
		if(!$canWrite){
			$this->_warning('This context is read-only.');
		}
		$objects = $db->listItems($context['objects']);
		$attributes = $db->listItems($context['attributes']);
		$model['page'] = 'relations';
		$model['data'] = array(
			'context' => $context,
			'title' => $model['title'],
			'relations' => $relations,
			'objects' => $objects,
			'attributes' => $attributes,
			'filterObject' => $filterObject,
			'filterAttribute' => $filterAttribute,
			'filterHolds' => $filterHolds,
			'filterObjectHaving' => $filterObjectHaving,
			'filterAttributeHaving' => $filterAttributeHaving,
			'filterObjectMatches' => $filterObjectMatches,
			'filterAttributeMatches' => $filterAttributeMatches,
			'canWrite' => $canWrite,
			'statistics' => $stats,
			'limit' => 5000

		);
		$v = new MVC_Viewer($model);
		$v->render('layout');
	}

	public function doexportAction(){
		$contextId = $this->getParam('context');
		$db = DataAccess::instance();
		header("Content-type: text/csv;charset=utf-8");
		$db->streamContext($contextId, array($this, 'spitCSV'));
	}

	public function exportAction(){
		$model = $this->model();
		$db = DataAccess::instance();
		$model['page'] = 'export-context';
		$model['title'] = 'Export context';
		$model['data'] = array(
				'contexts' => $db->listContexts()
		);
		$v = new MVC_Html($model);
		$v->render('layout');
	}

	public function spitCSV($o, $a, $h){
		print $o . ',' . $a . ',' . $h . "\n";
	}

	public function importAction(){
		$model = $this->model();
		$model['page'] = 'import-context';
		$model['title'] = 'Import context';
		$v = new MVC_Html($model);
		$v->render('layout');
	}

	public function doimportAction(){
		if($_SERVER['REQUEST_METHOD'] === 'POST'){
			$validators = array(
				'contextName' => array('NotEmpty'),
				'objectTypeName' => array('NotEmpty'),
				'attributeTypeName' => array('NotEmpty'),
				'file' => array('NotEmpty'),
			);
			$sameAs = FALSE;
			if($this->has('attributeTypeSameAsObjectType') && $this->get('attributeTypeSameAsObjectType') === 'checked'){
				unset($validators['attributeTypeName']);
				$sameAs = TRUE;
			}
			// validate input
			$errors = $this->validate($validators);
			if(!empty($errors)){
				error_malformed($errors);
			}
			$cName = $this->get('contextName');
			$otName = $this->get('objectTypeName');
			if($sameAs){
				$atName = NULL;
			}else{
				$atName = $this->get('attributeTypeName');
			}
			$file = $_FILES['file'];

			$db = DataAccess::instance();
			$contextId = $db->importContextFromCsv($file['tmp_name'], $cName, $otName, $atName);
			// Grant WRITE access to imported context
			ACL::grantUser($this->_userid(), ACL::P_WRITE, ACL::CONTEXT, $contextId);
			ACL::grantUser($this->_userid(), ACL::P_GRANT, ACL::CONTEXT, $contextId);
			// Grant on types
			$context = $db->getContext($contextId);
			ACL::grantUser($this->_userid(), ACL::P_WRITE, ACL::TYPE, $context['objects']);
			ACL::grantUser($this->_userid(), ACL::P_WRITE, ACL::TYPE, $context['attributes']);
			// Fix status of non defined relations to NOT HOLDS
			//$db->bulkHolds($contextId, NULL, 0);
				
			$this->_success('Context ' . $cName . ' has been imported.');
			header('Location: ' . route('contexts','context', array('context' => $contextId)));
		}else{
			error_method_not_allowed($_SERVER['REQUEST_METHOD']);
		}
	}

	public function taxonomyAction(){
		$contextId = $this->getParam('context');
		$db = DataAccess::instance();
		
		// Everybody can create a taxonomy out of a context
		
		// 1. write the attribute extents to a list
		$objects = $db->listAllExtents($contextId);
//  		print '<pre>';
//  		print "\nExtents: " . print_r($objects, TRUE);
		// 2. Compute all pairwise intersections
		$cycle = TRUE;
		while($cycle){
			$cycle = FALSE;
			foreach($objects as $obj){
				foreach($objects as $obj2){
					$intersect = array_intersect($obj, $obj2);
					sort($intersect);
					if( !in_array($intersect, $objects) ){
						array_push($objects, $intersect);
						$cycle = TRUE;
					}
				}
			}
		}
		// Add G
		$G = array();
		foreach($objects as $arr){
			$G = array_merge($arr, $G);
		}
		$G = array_unique($G);
		sort($G);
		if(!in_array($G, $objects)){
			array_push($objects, $G);
		}
		// empty extent
		//array_push($objects, array());
		$extents = $objects;
		sort($extents);
		$concepts = array();
		// 3. Compute the intents
// 		$attributes = $db->intent($contextId);
		$allattributes = $db->attributesInContext($contextId);
		sort($allattributes);
		$allobjects = $db->objectsInContext($contextId);
		sort($allobjects);
//  		print "\nObjects: " . join(',',$allobjects);
//  		print "\nAttributes: " . join(',',$allattributes);
		foreach($extents as $extent){
			$intent = $db->intent($contextId, $extent);
			if(! (empty($extent) && empty($intent))){
				if($intent == $allattributes || $extent == array()){
					$topconceptfound = TRUE;
				}
				if($extent == $allobjects || $intent == array()){
					$bottomconceptfound = TRUE;
				}
				array_push($concepts, array($extent, $intent));
			}
		}
//  		print "\nConcepts: " . print_r($concepts, TRUE);
		if(!isset($topconceptfound)){
			// Add empty extent (only if needed!)
//  			print "\nAdd empty extent. ";
			array_push($concepts, array(array(), $allattributes));
		}
		if(!isset($bottomconceptfound)){
			// Add empty intent (only if needed!)
//  			print "\nAdd empty intent";
			array_push($concepts, array($allobjects, array()));
		}
  		
		// Compute the relation "is subconcept of"
		$subrelations = array();
		foreach($concepts as $c1key => $concept1){
			foreach($concepts as $c2key => $concept2){
				if($concept1 == $concept2){
					// Do nothing
				}else // We compute the intersection of extents
				if ( array_intersect($concept1[0], $concept2[0]) == $concept1[0] ){
					// concept1 is sub concept of concept2
					if(!isset($subrelations[$c1key])){
						$subrelations[$c1key] = array();
					}
					$skip = FALSE;
					foreach($subrelations[$c1key] as &$superc){
						// If c2 is subconcept of an already visited superconcept, replace the old one with c2
						if(array_intersect($concept2[0], $concepts[$superc][0]) == $concept2[0]){
							$superc = $c2key;
							//print "\n " . print_r($concept2[0], true) . " is subconcept of " . print_r($concepts[$superc][0], true);
						}
						
						// If c2 is superconcept of a visited concept, ignore it
						if(array_intersect($concepts[$superc][0], $concept2[0]) == $concepts[$superc][0]){
							$skip = TRUE;
							//print "\n " . print_r($concept2[0], true) . " is superconcept of " . print_r($concepts[$superc][0], true);
						}
						
					}
					// If it is not there, add it
					if(!$skip && !in_array($c2key, $subrelations[$c1key])){
						array_push($subrelations[$c1key], $c2key);
					}
				}
			}
		}
		
		// XXX If there is only 1 concept we link it to itself
		if(empty($subrelations) && count($concepts) == 1){
			$subrelations[0] = array(0);
		}
// 		print "\nConcepts:";
//  		print_r($concepts);
// 		print "\nSubrelations:";
//  		print_r($subrelations);
//  		print '</pre>';
//  		die;
		// Build graph from subrelations
		$taxonomy = array();
		foreach($subrelations as $ck => $sckeys){
			foreach($sckeys as $sck){
				$rel = array($concepts[$ck], $concepts[$sck]);
				if (! in_array ( $rel, $taxonomy )) {
					array_push ( $taxonomy, $rel );
				}
			}
		}
		
		$taxonomyId = $db->createTaxonomy($contextId, $taxonomy);
		$taxonomy = $db->taxonomy($taxonomyId);
		// ACL
		ACL::grantUser($this->_userid(), ACL::P_WRITE, ACL::TAXONOMY, $taxonomyId);
		ACL::grantUser($this->_userid(), ACL::P_GRANT, ACL::TAXONOMY, $taxonomyId);
		$this->_success('Taxonomy ' . $taxonomy['name'] . ' has been generated.');
		redirect(route('taxonomies', 'taxonomy', array('taxonomy' => $taxonomyId)));
	}
	
	function deleteAction(){
		$contextId = $this->getParam('context'); // the contextId
		$db = DataAccess::instance();
		$canWrite = ACL::allowed($this->_userid(), ACL::P_WRITE, ACL::CONTEXT, $contextId);
		if(!$canWrite){
			$this->_forbidden('Cannot write context');
			return;
		}
		$contextName = $db->getContextName($contextId);
		$model = $this->model();
		$this->_danger('Are you sure?');
		$this->_info('This will not delete any derived taxonomy.');
		$model['title'] = 'Delete context #' . $contextId . ': "' . $contextName . '"';
		$model['page'] = 'delete-context';
		$model['actions'] = $this->_contextActions($canWrite);
		$model['actions_select'] = 'Delete';
		$model['data'] = array('contextId' => $contextId, 'contextName' => $contextName);
		$v = new MVC_Viewer($model);
		$v->render('layout');
	}
	
	function postdeletecontextAction(){
		if(!ACL::allowed($this->_userid(), ACL::P_WRITE, ACL::CONTEXT, $this->get('delete'))){
			$this->_forbidden('Cannot edit context');
			return;
		}
		// only POST
		if($_SERVER['REQUEST_METHOD'] === 'POST'){
			$id = $this->get('delete');
			$db = DataAccess::instance();
			$db->deleteContext($id);
			$this->_success('The context has been deleted.');
			redirect(route('contexts', 'list'));
		}else{
			$this->_forbidden();
		}
	}
}


class HoldsSan implements MVC_Sanitizer{
	public function filter($val){
		if($val === 'true'){
			return 1;
		}
		if($val === 'false'){
			return 0;
		}
		return -1;
	}
};