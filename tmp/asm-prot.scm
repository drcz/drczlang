;; jeszcze coś: zauważ drogi derczyku, że w tym kodzie nontoper się powtarza cond
;; co rozłazi się po kodzie ze wzlgędu na jego rozgałęźniki LDF, SEL i SER.
;; wyekstra-chuj go w niedługiej przyszłości!

(require-extension (srfi 1))

(define (assemble code)
  (cons (length (all-symbols-in code)) ;; małe novum: kod zaczyna się od ilości użytych symboli!
   (ops2opcodes (symbols2addresses (append '(LDC T BIND T) code)))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (ops2opcodes code)
  (cond ((eq? code '()) '())
	((eq? (car code) 'LDF) (append (list (opcode 'LDF) (ops2opcodes (cadr code)))
				       (ops2opcodes (cddr code))))
	((eq? (car code) 'SEL) (append (list (opcode 'SEL)
					     (ops2opcodes (cadr code))
					     (ops2opcodes (caddr code)))
				       (ops2opcodes (cdddr code))))
	((eq? (car code) 'SER) (append (list (opcode 'SER)
					     (ops2opcodes (cadr code))
					     (ops2opcodes (caddr code)))
				       (ops2opcodes (cdddr code))))	
	(else (append (cons (opcode (car code))
			    (cdr (first-command code)))
		      (ops2opcodes (rest-commands code))))))


(define (op-arity op)
  (cond ((eq? op 'SEL) 2)
	((eq? op 'SER) 2)
	((eq? op 'LDC) 1)
	((eq? op 'LDF) 1)
	((eq? op 'LU) 1)
	((eq? op 'BIND) 1)
	((eq? op 'UNBIND) 1)
	(else 0)))


(define (first-command code)
  (if (eq? code '()) '() (take code (1+ (op-arity (car code))))))

(define (rest-commands code)
  (if (eq? code '()) '() (drop code (1+ (op-arity (car code))))))


(define (opcode op)
  (cond ((eq? op 'LDC) 0)
	((eq? op 'LDF) 0)

	((eq? op 'LU) 1)
	((eq? op 'BIND) 2)
	((eq? op 'UNBIND) 3)

	((eq? op 'SEL) 4)
	((eq? op 'SER) 5)
	((eq? op 'AP) 6)
	((eq? op 'TRAP) 7)
	((eq? op 'RTN) 8)

	((eq? op 'EQ) 9)
	((eq? op 'NUM) 10)
	((eq? op 'ATOM) 11)
	((eq? op 'CAR) 12)
	((eq? op 'CDR) 13)
	((eq? op 'CONS) 14)

	((eq? op 'DISP) 15)
	((eq? op 'READ) 16)

	((eq? op 'ADD) 17)
	((eq? op 'SUB) 18)
	((eq? op 'MUL) 19)
	((eq? op 'DIV) 20)
	((eq? op 'MOD) 21)
	((eq? op 'GT) 22)
	((eq? op 'LT) 23)
	(else 'ERR)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (symbols2addresses code)
  (syms2addr code (all-symbols-in code)))

(define (syms2addr code symlist)
  (cond ((eq? code '()) '())
	((eq? (car code) 'LDF) (append (list 'LDF (syms2addr (cadr code) symlist))
				       (syms2addr (rest-commands code) symlist)))
	((eq? (car code) 'SEL) (append (list 'SEL
					     (syms2addr (cadr code) symlist)
					     (syms2addr (caddr code) symlist))
				       (syms2addr (rest-commands code) symlist)))
	((eq? (car code) 'SER) (append (list 'SER
					     (syms2addr (cadr code) symlist)
					     (syms2addr (caddr code) symlist))
				       (syms2addr (rest-commands code) symlist)))
	((member (car code) '(LU BIND UNBIND))
	 (append (list (car code) (position (cadr code) symlist))
		 (syms2addr (rest-commands code) symlist)))
	(else (append (first-command code)
		      (syms2addr (rest-commands code) symlist)))))

(define (all-symbols-in code)
  (delete-duplicates (cons 'T (gather-symbols code '()))))

(define (gather-symbols code seen)
  (cond ((eq? code '()) seen)
	((eq? (car code) 'LDF) (append (gather-symbols (cadr code) seen)
				       (gather-symbols (rest-commands code) seen)))
	((member (car code) '(SEL SER)) (append (gather-symbols (cadr code) seen)
						(gather-symbols (caddr code) seen)
						(gather-symbols (rest-commands code) seen)))
	((member (car code) '(LU BIND UNBIND)) (gather-symbols (cddr code) (cons (cadr code) seen)))
	(else (gather-symbols (rest-commands code) seen))))

(define (position elem thelist)
  (list-index (lambda (x) (eq? elem x)) thelist))

;;;; go vege go:

;; f(x,y)=cons(x,cons(x,y))
;(display (assemble '(LDF (BIND x BIND y LU x LU x CONS LU y CONS UNBIND x UNBIND y RTN) BIND f LDC 2 LDC 3 LU f AP)))

;; phiii.
;(display (assemble '(LDF (BIND N LDC 0.0 LU N EQ SEL (LDC 1.0 RTN) (LDC 1.0 LU N SUB LU S AP LU N MUL RTN) UNBIND N RTN) BIND S LDC 5.0 LU S AP)))

;(display (assemble '(LDC 2 LDC 3 EQ SEL (LDC niekons RTN) (LDC kons RTN) LDC 997 CONS)))

(define id1 '(LDC (_cons _car _cdr _num _atom = DISP READ + - * / % > <) BIND PRIMOPNAMES LDF (BIND SIDEEFFECT BIND ENV LU ENV LU ENV READ LU EVAL AP DISP LU REPL AP UNBIND SIDEEFFECT UNBIND ENV RTN) BIND REPL LDF (BIND EXPR BIND ENV LDC () LU EXPR EQ SEL (LU EXPR RTN) (LU EXPR NUM SEL (LU EXPR RTN) (LU EXPR ATOM SEL (LU ENV LU EXPR LU LOOKUP AP RTN) (LDC QUOTE LU EXPR CAR EQ SEL (LU EXPR CDR CAR RTN) (LDC ^ LU EXPR CAR EQ SEL (LU EXPR RTN) (LDC ! LU EXPR CAR EQ SEL (LU ENV LU ENV LU EXPR CDR CDR CAR LU EVAL AP LU EXPR CDR CAR LU UPDATE-ENV AP RTN) (LDC IF LU EXPR CAR EQ SEL (LU ENV LU EXPR CDR CDR CDR CAR LU EXPR CDR CDR CAR LU EXPR CDR CAR LU EVAL-IF AP RTN) (LDC ? LU EXPR CAR EQ SEL (LU ENV LU EXPR CDR LU EVAL-COND AP RTN) (LU ENV LU ENV LU EXPR CDR LU EVLIS AP LU EXPR CAR LU APPLY AP RTN) RTN) RTN) RTN) RTN) RTN) RTN) RTN) UNBIND EXPR UNBIND ENV RTN) BIND EVAL LDF (BIND PROC BIND LIST LDC () LU LIST EQ SEL (LDC () RTN) (LU LIST CDR LU PROC LU MAP AP LU LIST CAR LU PROC AP CONS RTN) UNBIND PROC UNBIND LIST RTN) BIND MAP LDF (BIND A BIND B LDC () LU A EQ SEL (LU B RTN) (LU B LU A CDR LU APPEND AP LU A CAR CONS RTN) UNBIND A UNBIND B RTN) BIND APPEND LDF (BIND A BIND B LDC () LU A EQ SEL (LDC () RTN) (LU B CDR LU A CDR LU PAIR AP LU B CAR LU A CAR CONS CONS RTN) UNBIND A UNBIND B RTN) BIND PAIR LDF (BIND EL BIND LIST LDC () LU LIST EQ SEL (LDC () RTN) (LU LIST CAR LU EL EQ SEL (LU T RTN) (LU LIST CDR LU EL LU MEMBER? AP RTN) RTN) UNBIND EL UNBIND LIST RTN) BIND MEMBER? LDF (BIND LIST BIND ENV LDC () LU LIST EQ SEL (LDC () RTN) (LU ENV LU LIST CDR LU EVLIS AP LU ENV LU LIST CAR LU EVAL AP CONS RTN) UNBIND LIST UNBIND ENV RTN) BIND EVLIS LDF (BIND KEY BIND ALIST LDC () LU ALIST EQ SEL (LDC () RTN) (LU ALIST CAR CAR LU KEY EQ SEL (LU ALIST CAR CDR RTN) (LU ALIST CDR LU KEY LU LOOKUP AP RTN) RTN) UNBIND KEY UNBIND ALIST RTN) BIND LOOKUP LDF (BIND CONDITION BIND BRANCH1 BIND BRANCH2 BIND ENV LU ENV LU CONDITION LU EVAL AP SEL (LU ENV LU BRANCH1 LU EVAL AP RTN) (LU ENV LU BRANCH2 LU EVAL AP RTN) UNBIND CONDITION UNBIND BRANCH1 UNBIND BRANCH2 UNBIND ENV RTN) BIND EVAL-IF LDF (BIND CONDLIST BIND ENV LDC () LU CONDLIST EQ SEL (LDC () RTN) (LU ENV LU CONDLIST CAR CAR LU EVAL AP SEL (LU ENV LU CONDLIST CAR CDR CAR LU EVAL AP RTN) (LU ENV LU CONDLIST CDR LU EVAL-COND AP RTN) RTN) UNBIND CONDLIST UNBIND ENV RTN) BIND EVAL-COND LDF (BIND OP BIND ARGS BIND ENV LDC () LU OP EQ SEL (LDC (ERR: APPLY (NIL)) RTN) (LU OP NUM SEL (LDC (ERR: APPLY (NUMBER)) RTN) (LU OP LU IS-PRIMOP? AP SEL (LU ARGS LU OP LU APPLY-PRIM AP RTN) (LU OP ATOM SEL (LU ENV LU ARGS LU ENV LU OP LU EVAL AP LU APPLY AP RTN) (LDC ^ LU OP CAR EQ SEL (LU ENV LU ARGS LU OP CDR CDR CAR LU OP CDR CAR LU APPLY-LAMBDA AP RTN) (LDC (ERR: APPLY) RTN) RTN) RTN) RTN) RTN) UNBIND OP UNBIND ARGS UNBIND ENV RTN) BIND APPLY LDF (BIND SYM LU PRIMOPNAMES LU SYM LU MEMBER? AP UNBIND SYM RTN) BIND IS-PRIMOP? LDF (BIND OP BIND ARGS LDC _cons LU OP EQ SEL (LU ARGS CDR CAR LU ARGS CAR CONS RTN) (LDC _car LU OP EQ SEL (LU ARGS CAR CAR RTN) (LDC _cdr LU OP EQ SEL (LU ARGS CAR CDR RTN) (LDC _num LU OP EQ SEL (LU ARGS CAR NUM RTN) (LDC _atom LU OP EQ SEL (LU ARGS CAR ATOM RTN) (LDC = LU OP EQ SEL (LU ARGS CDR CAR LU ARGS CAR EQ RTN) (LDC READ LU OP EQ SEL (READ RTN) (LDC DISP LU OP EQ SEL (LU ARGS CAR DISP RTN) (LDC + LU OP EQ SEL (LU ARGS CDR CAR LU ARGS CAR ADD RTN) (LDC - LU OP EQ SEL (LU ARGS CDR CAR LU ARGS CAR SUB RTN) (LDC * LU OP EQ SEL (LU ARGS CDR CAR LU ARGS CAR MUL RTN) (LDC / LU OP EQ SEL (LU ARGS CDR CAR LU ARGS CAR DIV RTN) (LDC % LU OP EQ SEL (LU ARGS CDR CAR LU ARGS CAR MOD RTN) (LDC > LU OP EQ SEL (LU ARGS CDR CAR LU ARGS CAR GT RTN) (LDC < LU OP EQ SEL (LU ARGS CDR CAR LU ARGS CAR LT RTN) (LDC (ERR (UNKNOWN PRIMOP)) RTN) RTN) RTN) RTN) RTN) RTN) RTN) RTN) RTN) RTN) RTN) RTN) RTN) RTN) RTN) UNBIND OP UNBIND ARGS RTN) BIND APPLY-PRIM LDF (BIND ARGLIST BIND BODY BIND ARGVALS BIND ENV LU ENV LU ARGVALS LU ARGLIST LU PAIR AP LU APPEND AP LU BODY LU EVAL AP UNBIND ARGLIST UNBIND BODY UNBIND ARGVALS UNBIND ENV RTN) BIND APPLY-LAMBDA LDF (BIND SYM BIND VAL BIND ENV LU ENV LU VAL LU SYM CONS CONS LU VAL DISP LU REPL AP UNBIND SYM UNBIND VAL UNBIND ENV RTN) BIND UPDATE-ENV LDF (LU PRIMOPNAMES LU PRIMOPNAMES LU PAIR AP LDC T LDC T CONS CONS LDC (- DRCZ0 SŁUCHA -) DISP LU REPL AP RTN) BIND RUN LU RUN AP))

;(display (all-symbols-in id1))
;(newline)

;(display (assemble id1))
(display (assemble '(READ LDC () CONS)))
(newline)
