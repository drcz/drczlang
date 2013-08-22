;; tak na szybciocha... ;;;;;;;;;;;;;;;
;; COMP<FCL*->php>, 2013-08-22, drcz ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(use-modules (srfi srfi-1)
	     (srfi srfi-13)
	     (srfi srfi-31)
	     (ice-9 match))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (PROGRAM:input p) (car p))
(define (PROGRAM:start-pp p) (cadr p))
(define (PROGRAM:block-map p) (cddr p))

(define (BLOCK:code bl) (cdr bl))
(define (BLOCK:label bl) (car bl))

(define (BLOCKMAP:find label bl)
  (cond ((null? bl) #f)
	((eq? (BLOCK:label (car bl)) label) (BLOCK:code (car bl)))
	(else (BLOCKMAP:find label (cdr bl)))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (sym->phpvar s) (string-join (list "$" (symbol->string s)) ""))

(define (list->arr l)
  (string-join (list "array("
		       (string-join (map const->php l) ",")
		       ")")
	       ""))

(define (const->php expr)
  (match expr
    (() "array()")
    ((? number? n) (number->string n))
    ((? symbol? s) (string-join (list "'" (symbol->string s) "'") ""))
    ((h . t) (list->arr expr))
    (otherwise (throw 'const-2-php-error expr))))

;(const->php '(qwe (1 2 3) (4 5 6)))

;; pfff.
(define (seq n)
  (let loop ((i 1))
    (if (> i n) '() (cons i (loop (+ i 1))))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (gather-calls prog)
  (let loop ((pend (map cddr
			(filter (lambda (x) (eq? (car x) 'let))
				(append-map BLOCK:code
					    (PROGRAM:block-map prog)))))
	     (res '()))
    (match pend
      (() (delete-duplicates res))
      ((() . pend) (loop pend res))
      (((()) . pend) (loop pend res))
      ((((())) . pend) (loop pend res))
      ((((? number?)) . pend) (loop pend res))
      ((((? symbol?)) . pend) (loop pend res))
      ((((quote _)) . pend) (loop pend res))
      (((('call label . env)) . pend) (loop pend (cons (cons label env) res)))
      ((((op . args)) . pend) (loop (append (map list args) pend) res))
      (otherwise (throw 'syntax-error-in-let-rhs (car pend))))))

(define (goto-if-closure label bl)
  (let loop ((pend (list label))
	     (res  (list label)))
    (if (null? pend)
	(reverse res)
	(let* ((code (BLOCKMAP:find (car pend) bl))
	       (last-op (last code))
	       (pend (cdr pend))
	       (children (match last-op
			   (('goto l) (list l))
			   (('if _ l1 l2) (list l1 l2))
			   (('return _) '())))
	       (new-children (filter (lambda (c) (not (member c res)))
				     children)))
	  (loop (append new-children pend)
		(append new-children res))))))
	     
(define (mk-subprocedure labels bl)
  (map (lambda (l) (cons l (BLOCKMAP:find l bl))) labels))

(define (compile p)
  (let* ((start-label (PROGRAM:start-pp p))
	 (inputs (PROGRAM:input p))
	 (initcall (cons start-label inputs))			 
	 (calls (delete-duplicates (cons initcall (gather-calls p))))
	 (blocks (PROGRAM:block-map p))
	 (header (list "<?php\nrequire_once('fclops.php');\n"))
	 (footer (list "\n?>\n"))
	 (defs (map (lambda (call)
		      (compile-subprocedure (car call) (cdr call) blocks))
		    calls))
	 (starter (list "echo json_encode(proc_"
			(symbol->string start-label)
			"("
			(string-join (map (lambda (i) 
					    (string-join (list "json_decode($argv["
							       (number->string i)
							       "])")
							 ""))
					  (seq (length inputs)))
				     ",")
			")) . \"\\n\";")))
    (string-join (append header
			 defs
			 starter
			 footer)
		 "")))

(define (compile-subprocedure label env bl)
  (string-join (append (list "function proc_" (symbol->string label) "(")
		       (list (string-join (map sym->phpvar env) ","))
		       (list ") {\n")
		       (map comp-block
			    (mk-subprocedure (goto-if-closure label
							      bl)
					     bl))
		       (list "\n}\n"))
	       ""))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; [(let x e)] -> $x=[e];
; [(goto l)] -> goto l;
; [(if e l1 l2)] -> if([e]) goto l1; else goto l2;
; [(+ e1 e2)] -> ([e1] + [e2])
; [(* e1 e2)] -> ([e1] * [e2])
; [(= e1 e2)] -> ([e1] == [e2])
; [(< e1 e2)] -> ([e1] < [e2])
; [(and e1 e2)] -> ([e1] && [e2])
; [(or e1 e2)] -> ([e1] || [e2])
; [(not e)] -> !([e])

; ... :)

(define (mk-inits vars n)  
  (if (null? vars)
      '()
      (cons (string-join (list "$"
			       (symbol->string (car vars))
			       "=$argv["
			       (number->string n)
			       "];")
			 "")
	    (mk-inits (cdr vars) (+ n 1)))))

(define (comp-block bl)
  (string-join (append (cons (string-join (list (symbol->string (BLOCK:label bl)) ":") "")
			     (map comp (BLOCK:code bl)))
		       '("\n"))
	       " "))

(define (comp expr)
  (match expr
    (('let x expr) (string-join (list "$"
                                     (symbol->string x)
                                     "="
				     (comp-expr expr)
				     ";")
			       ""))
    (('goto l) (string-join (list "goto "
				  (symbol->string l)
				  ";")
			    ""))
    (('if e l1 l2) (string-join (list "if"
				  (comp-expr e)
				  " goto "
				  (symbol->string l1)
				  "; else goto "
				  (symbol->string l2)
				  ";")
			    ""))
    (('return e) (string-join (list "return "
				    (comp-expr e)
				    ";")))
    (otherwise (throw 'unknown-operation-error expr))))

(define (comp-expr expr)
  (match expr
    (() "array()")
    ((? number? n) (number->string n))
    ((? symbol? s) (string-join (list "$"
				      (symbol->string s))
				""))
    (('quote e) (const->php e))
    (('call l . env) (string-join (list "proc_"
					(symbol->string l)
					"("
				        (string-join (map (lambda (v)
							    (string-join (list "$"
									       (symbol->string v))
									 ""))
							  env)
						     ",")
					")")
				  ""))
    (('cons e1 e2) (string-join (list "SE_cons("
				      (comp-expr e1)
				      ","
				      (comp-expr e2)
				      ")")
				""))
    (('car e) (string-join (list "SE_car("
				 (comp-expr e)
				 ")")
			     ""))
    (('cdr e) (string-join (list "SE_cdr("
				 (comp-expr e)
				 ")")
			     ""))
    (('list . l) (string-join (list "array("
				    (string-join (map comp-expr l)
						 ",")
				    ")")
			      ""))
    (('+ e1 e2) (string-join (list "("
				   (comp-expr e1)
				   "+"
				   (comp-expr e2)
				   ")")
			     ""))
    (('- e1 e2) (string-join (list "("
				   (comp-expr e1)
				   "-"
				   (comp-expr e2)
				   ")")
			     ""))
    (('* e1 e2) (string-join (list "("
				   (comp-expr e1)
				   "*"
				   (comp-expr e2)
				   ")")
			     ""))
    (('= e1 e2) (string-join (list "("
				   (comp-expr e1)
				   "=="
				   (comp-expr e2)
				   ")")
			     ""))
    (('< e1 e2) (string-join (list "("
				   (comp-expr e1)
				   "<"
				   (comp-expr e2)
				   ")")
			     ""))
    (('and e1 e2) (string-join (list "("
				   (comp-expr e1)
				   "&&"
				   (comp-expr e2)
				   ")")
			     ""))
    (('or e1 e2) (string-join (list "("
				   (comp-expr e1)
				   "||"
				   (comp-expr e2)
				   ")")
			     ""))
    (('not e) (string-join (list "!" (comp-expr e))))
    (otherwise (throw 'unknown-operator-error expr))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;; eg

(define pow '((n m)
	      init
	      (init (let r 1)
		    (goto test))
	      (test (if (= n 0) stop loop))
	      (loop (let r (* r m))
		    (let n (- n 1))
		    (goto test))
	      (stop (return r))))

;(write "pow:")(newline)
(display (compile pow))
;(newline)
 
(define apd '((a b)
	      init

	      (init (let res (call apd a b))
		    (return res))

	      (apd (if (= a (quote ())) end loop))

	      (loop (let next (car a))
		    (let a (cdr a))
		    (let r (call apd a b))
		    (let res (cons next r))
		    (return res))

	      (end (return b))))

;(write "apd:")(newline)
(display (compile apd))
;(newline)

(define ack '((m n)
	      ack

	      (ack (if (= m 0) done next))
	      (done (return (+ n 1)))
	      (next (if (= n 0) ack0 ack1))
	      (ack0 (let n 1)
		    (goto ack2))
	      (ack1 (let n (- n 1))
		    (let n (call ack m n))
		    (goto ack2))
	      (ack2 (let m (- m 1))
		    (let n (call ack m n))
		    (return n)) ))

;(write "ack:")(newline)
(display (compile ack))
;(newline)
