<?php
require_once 'default.inc';
class TaxonomiesController extends DefaultController {
	public function model() {
		$model = parent::model ();
		$model ['title'] = 'Taxonomies';
		return $model;
	}
	public function indexAction() {
		$this->listAction ();
	}
	
	public function listAction() {
		$db = DataAccess::instance ();
		$t = $db->taxonomies ($this->_userid());
		$model = $this->model ();
		$model ['title'] = 'Taxonomies';
		$model ['page'] = 'taxonomies';
		$model ['data'] = array (
			'taxonomies' => $t 
		);
		$v = new MVC_Html ( $model );
		$v->render ( 'layout' );
	}
	
	function deleteAction(){
		$taxonomyId = $this->getParam('taxonomy'); // the taxonomyId
		$db = DataAccess::instance();
		$taxonomy = $db->taxonomy($taxonomyId);
		$model = $this->model();
		$this->_danger('Are you sure?');
		$this->_info('This will not delete the source context.');
		$model['title'] = 'Delete taxonomy #' . $taxonomyId . ': "' . $taxonomy['name'] . '"';
		$model['page'] = 'delete-taxonomy';
		$model['actions_select'] = 'Delete';
		$model['data'] = array('taxonomyId' => $taxonomyId, 'taxonomyName' => $taxonomy['name']);
		$v = new MVC_Viewer($model);
		$v->render('layout');
	}
	
	function postdeletetaxonomyAction(){
		if(!ACL::allowed($this->_userid(), ACL::P_WRITE, ACL::TAXONOMY, $this->get('delete'))){
			$this->_forbidden('Cannot edit taxonomy ');
			return;
		}
		// only POST
		if($_SERVER['REQUEST_METHOD'] === 'POST'){
			$id = $this->get('delete');
			$db = DataAccess::instance();
			$db->deleteTaxonomy($id);
			$this->_success('The taxonomy has been deleted.');
			redirect(route('taxonomies', 'list'));
		}else{
			$this->_forbidden();
		}
	}
	
	public function taxonomyAction() {
		$this->setParam ( 'viewas', 'table' );
		$this->_taxonomy ();
	}
	
	public function d3inputAction() {
		$errs = $this->validate ( array (
				'taxonomy' => array (
						'NotEmpty',
						'Int' 
				) 
		) );
		if (count ( $errs ) > 0) {
			error_malformed ( $errs );
		}
		$taxonomyId = $this->getParam ( 'taxonomy' );
		$db = DataAccess::instance ();
		$taxonomy = $db->taxonomy ( $taxonomyId );
		$concepts = $db->concepts ( $taxonomyId );
		
		$json = array ();
		$json ['nodes'] = array ();
		
		$nodes = array ();
		$links = array ();
		$nodeIndex = array ();
		$idx = 0;
		foreach ( $concepts as $con ) {
			if (count ( $con ['top'] ) == 0) {
				$group = 3;
				$role = 't';
			} elseif (count ( $con ['bottom'] ) == 0) {
				$group = 2;
				$role = 'b';
			} else {
				$group = 1;
				$role = 'c';
			}
			
			$node = array (
					'name' => ($con ['name']) ? $con ['name'] : strval ( $con ['id'] ),
					'id' => $con ['id'],
					'group' => $group,
					'role' => $role,
					'top' => $con ['top'],
					'bottom' => $con ['bottom'],
					'extent' => explode(',', $con['extent']),
					'intent' => explode(',', $con['intent'])
			);
			array_push ( $nodes, $node );
			$nodeIndex [$con ['id']] = $idx;
			$idx ++;
			foreach ( $con ['bottom'] as $l ) {
				array_push ( $links, array (
						'source' => strval ( $con ['id'] ),
						'target' => $l,
						'value' => 1 
				) );
			}
		}
		// Override links
		$newlinks = array ();
		foreach ( $links as $link ) {
			$newlink = array (
					'source' => $nodeIndex [$link ['source']],
					'target' => $nodeIndex [$link ['target']],
					'value' => $link ['value'] 
			);
			array_push ( $newlinks, $newlink );
		}
		// Override top/bottom
		foreach ( $nodes as &$node ) {
			foreach ( $node ['top'] as &$top ) {
				$top = $nodeIndex [$top];
			}
			foreach ( $node ['bottom'] as &$bot ) {
				$bot = $nodeIndex [$bot];
			}
		}
		$json = array ();
		$json ['nodes'] = $nodes;
		$json ['links'] = $newlinks;
		header ( 'Content-type: application/json; charset=utf8' );
		print json_encode ( $json );
	}
	
	private function _taxonomyActions($canWrite){
		return array(
				'List' => array('href' => route('taxonomies', 'taxonomy', array('taxonomy' => $this->getParam('taxonomy'))), 'icon' => 'list', 'type' => 'primary'),
				'Lattice' => array( 'href' => route('taxonomies', 'lattice', array('taxonomy' => $this->getParam('taxonomy'))), 'icon' =>'picture', 'type' => 'primary'),
				'Delete' => array( 'href' => ($canWrite) ?  route('taxonomies', 'delete', array('taxonomy' => $this->getParam('taxonomy'))) : '', 'icon' => 'trash', 'type' => 'danger' )
		);
	}
	
	private function _taxonomy() {
		$errs = $this->validate ( array (
				'taxonomy' => array (
						'NotEmpty',
						'Int' 
				),
				'viewas' => array (
						new MVC_OneOfVal ( array (
								'lattice2',
								'table' 
						) ) 
				) 
		) );
		if (count ( $errs ) > 0) {
			error_malformed ( $errs );
		}
		$taxonomyId = $this->getParam ( 'taxonomy' );
		
		$canWrite = ACL::allowed($this->_userid(), ACL::P_WRITE, ACL::TAXONOMY, $taxonomyId);
		if(!$canWrite){
			$this->_warning('This taxonomy is read-only');
		}
		$db = DataAccess::instance ();
		$taxonomy = $db->taxonomy ( $taxonomyId );
		$concepts = $db->concepts ( $taxonomyId, false );
		$model = $this->model ();
		$model ['title'] = 'Taxonomy #' . $taxonomyId . ': ' . ($taxonomy ['name']);
		$model ['page'] = 'taxonomy';
		$model['actions'] = $this->_taxonomyActions($canWrite);
		$model ['data'] = array (
				'taxonomy' => $taxonomy,
				'concepts' => $concepts,
				'action' => $this->get ( $this->_action_parameter ),
				'viewas' => $this->get ( 'viewas' ) ,
				'canWrite' => $canWrite
		);
		if ($this->has ( 'concept' )) {
			$theconcept = $db->concept ( $this->get ( 'concept' ) );
			self::specifyFeatures($theconcept);
			$model ['data'] ['concept'] = $theconcept;
		}
		$v = new MVC_Html ( $model );
		$v->render ( 'layout' );
	}
	
	public function latticeAction() {
		$this->setParam ( 'viewas', 'lattice2' );
		$this->_taxonomy ();
	}
	
	public function postconceptAction() {
		$errs = $this->validate ( array (
				'taxonomy' => array (
						'NotEmpty',
						'Int' 
				),
				'concept' => array (
						'NotEmpty',
						'Int' 
				),
				'returnToAction' => array (
						'NotEmpty' 
				) 
		) );
		if (count ( $errs ) > 0) {
			error_malformed ( $errs );
		}
		$taxonomyId = $this->getParam ( 'taxonomy' );
		$conceptId = $this->getParam ( 'concept' );
		
		// ACL
		if(!ACL::allowed($this->_userid(), ACL::P_WRITE, ACL::TAXONOMY, $taxonomyId)){
			$this->actionNotAllowed();
		}
		
		$name = $this->getParam ( 'name' );
		$comment = $this->getParam ( 'comment' );
		
		$db = DataAccess::instance ();
		$ok = $db->updateConcept ( $conceptId, $name, $comment );
		if ($ok) {
			$this->_success('Concept updated.');
			redirect ( route ( 'taxonomies', $this->get ( 'returnToAction' ), array (
					'taxonomy' => $taxonomyId,
					'concept' => $conceptId 
			) ) );
		} else {
			throw new Exception ( 'Name update failed' );
		}
	}
	
	public function trashconceptAction() {
		$errs = $this->validate ( array (
				'taxonomy' => array (
						'NotEmpty',
						'Int' 
				) 
		) );
		if (count ( $errs ) > 0) {
			error_malformed ( $errs );
		}
		$taxonomyId = $this->getParam ( 'taxonomy' );
		
		// ACL
		if(!ACL::allowed($this->_userid(), ACL::P_WRITE, ACL::TAXONOMY, $taxonomyId)){
			$this->actionNotAllowed();
		}
		
		$db = DataAccess::instance ();
		
		// Cannot delete top and bottom!!!
		$forbid = array_merge ( $db->latticeTop ( $taxonomyId ), $db->latticeBottom ( $taxonomyId ) );
		$notIn = new MVC_OneOfVal ( $forbid );
		$notIn->forbidden ();
		$errs = $this->validate ( array (
				'concept' => array (
						'NotEmpty',
						'Int',
						$notIn 
				) 
		) );
		if (count ( $errs ) > 0) {
			error_malformed ( $errs );
		}
		// print "\n".'top: ' . $db->latticeTop($taxonomyId);
		// print "\nbottom: " . $db->latticeBottom($taxonomyId);
		// die;
		$conceptId = $this->getParam ( 'concept' );
		
		$allParents = $db->parents ( $taxonomyId );
		// remove node to delete (make broken path!)
		unset ( $allParents [$conceptId] );
		// Link tops to bottoms
		$parents = $db->taxonomyParents ( $conceptId );
		$children = $db->taxonomyChildren ( $conceptId );
		
		foreach ( $parents as $parent ) {
			$positives = array (); // XXX we optimize by reusing successful paths as 'bag of connected nodes'
			foreach ( $children as $child ) {
				$path = $this->_isConnected ( $allParents, array (
						$child 
				), $parent, $positives );
				if (empty ( $path )) {
					// path not found. Add link.
					$db->insertSubconcept ( $child, $parent, $taxonomyId );
				} else {
					// path found, do not add the link but remember the bag of connected nodes
					$positives = array_merge ( $positives, $path );
				}
			}
		}
		$ok = $db->deleteConcept ( $conceptId );
		if ($ok) {
			$this->_success('Concept deleted.');
			$args = array (
					'taxonomy' => $taxonomyId
			);
			if($this->get('focusOn') == 'child'){
				// take first child
				$child = array_pop($children);
				$args['concept'] = $child;
			}else if($this->get('focusOn') == 'parent'){
				// take first child
				$parent = array_pop($parents);
				$args['concept'] = $parent;
			}
			redirect ( route ( 'taxonomies', $this->get ( 'returnToAction' ), $args ) );
		} else {
			throw new Exception ( 'Name update failed' );
		}
	}
	
	private function _isConnected(array $links, $path, $to, array $positives = array()) {
		$from = $path [key ( array_slice ( $path, - 1, 1, TRUE ) )];
		if (! array_key_exists ( $from, $links )) {
			return array (); // empty path = not found
		}
		$nodes = $links [$from];
		if (in_array ( $to, $nodes )) {
			$newPath = $path;
			array_push ( $newPath, $to );
			return $newPath;
		} else {
			foreach ( $nodes as $node ) {
				$newPath = $path;
				array_push ( $newPath, $node );
				if (in_array ( $node, $positives )) {
					return $newPath;
				}
				$ret = $this->_isConnected ( $links, $newPath, $to, $positives );
				if (! empty ( $ret )) {
					return $ret;
				}
			}
		}
	}
	
	public static function specifyFeatures(&$concept) {
		if (strlen ( $concept ['extent'] ) > 0) {
			$extent = explode ( ",", $concept ['extent'] );
		} else {
			$extent = array ();
		}
		if (strlen ( $concept ['intent'] ) > 0) {
			$intent = explode ( ",", $concept ['intent'] );
		} else {
			$intent = array ();
		}
		
		$properIntent = $intent; // we'll remove things we find in top concepts
		foreach ( $concept ['top'] as $top ) {
			if (strlen ( $top ['intent'] ) !== 0) {
				$properIntent = array_diff ( $properIntent, explode ( ",", $top ['intent'] ) );
			}
		}
		
		$properExtent = $extent; // we'll remove things we find in top concepts
		foreach ( $concept ['bottom'] as $bot ) {
			if (strlen ( $bot ['extent'] ) !== 0) {
				$properExtent = array_diff ( $properExtent, explode ( ",", $bot ['extent'] ) );
			}
		}
		
		//
		if(!isset($properExtent)){
			$properExtent = array();
		}
		if(!isset($properIntent)){
			$properIntent = array();
		}
		// We prepare the inheritedIntent/inheritedExtent
		$inheritedExtent = array_diff ( $extent, $properExtent );
		$inheritedIntent = array_diff ( $intent, $properIntent );

		$concept['properIntent'] = $properIntent;
		$concept['properExtent'] = $properExtent;
		$concept['inheritedIntent'] = $inheritedIntent;
		$concept['inheritedExtent'] = $inheritedExtent;
	}
}