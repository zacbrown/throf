namespace FSThrof

module Interpreter =
    type State =
        {
        SymbolTable : Map<string, list<Parser.Node>>;
        Stack : list<Parser.Node>;
        ExecutionStream : list<Parser.Node>
        }

        member this.withNewStack(stack) =
            { SymbolTable = this.SymbolTable; ExecutionStream = this.ExecutionStream;
                Stack = stack }

    let getInterpreterState (state : State) =
        let symTable =
            Map.fold (fun acc key value -> acc + sprintf "%+A - %+A" key value) "" state.SymbolTable
        let stack =
            List.fold (fun acc elem -> sprintf "%+A " elem) "" state.Stack
        let stream =
            List.fold (fun acc elem -> sprintf "%+A " elem) "" state.ExecutionStream
        (symTable, stack, stream)

    let printInterpreterState (state : State) =
        let (symTable, stack, stream) = getInterpreterState state
        printfn "%+A" symTable
        printfn "%+A" stack
        printfn "%+A" stream

    exception MissingParseNodeInExecutionStream of string * string * string
    exception UnexpectedParserNode of string
    exception IntegerExpected of string
    exception NumericExpected of string
    exception InvalidOperation of string
    exception StackUnderflow of string * string

    let applyQuotationToStack (q : list<Parser.Node>) (state : State) =
        state.withNewStack <| (List.rev q) @ state.Stack

    module PrimitiveWords =
        let private raiseStackUnderflow (state : State) =
            let (_, stack, stream) = getInterpreterState state
            raise <| Errors.StackUnderflow(stack, stream)

        let ifWord (state : State) = 
            match state.Stack with
            | [] | [_ ; _] | [_] -> raiseStackUnderflow state
            | (predicate :: falseQuotation :: trueQuotation :: stack) ->
                match predicate with
                | Parser.Real 0.0 | Parser.Integer 0 | Parser.Boolean false ->
                    match falseQuotation with
                    | Parser.Quotation q -> applyQuotationToStack q state
                    | _ -> raise <| Errors.UnexpectedParserNode (sprintf "'if' must be provided quotations for true & false conditions, got: %+A" falseQuotation)
                | Parser.Word _ | Parser.Quotation _ | Parser.StringLiteral _ | Parser.Integer _ | Parser.Real _ | Parser.Boolean _ ->
                    match trueQuotation with
                    | Parser.Quotation q -> applyQuotationToStack q state
                    | _ -> raise <| Errors.UnexpectedParserNode (sprintf "'if' must be provided quotations for true & false conditions, got: %+A" trueQuotation)
                | _ -> raise <| Errors.UnexpectedParserNode (sprintf "%+A" predicate)

        let  dropWord (state : State) =
            match state.Stack with
            | [] -> raiseStackUnderflow state
            | _ -> state.withNewStack state.Stack.Tail

        let stackWord (state : State) =
            let rec printType (elem : Parser.Node) =
                match elem with
                | Parser.Integer value -> printfn "%d" value
                | Parser.Real value -> printfn "%f" value
                | Parser.Boolean value -> printfn "%b" value
                | Parser.StringLiteral value -> printfn "\"%s\"" value
                | Parser.Word value -> printfn "%s" value
                | Parser.Quotation value ->
                    printf "["
                    for x in value do
                        printType x
                        printfn " "
                    printfn "]"
                | _ -> raise <| Errors.UnexpectedParserNode (sprintf "The interpreter is in an unexpected state. The parse node '%+A' was unexpected on the stack." elem)
            printfn "Stack"
            printfn "============="
            for elem in state.Stack do
                printType elem
            printfn ""
            state

        let swapWord (state : State) =
            match state.Stack with
            | [] | [_] -> raiseStackUnderflow state
            | (first :: second :: rest) ->
                state.withNewStack (second :: first :: rest)

        let twoSwapWord (state : State) =
            match state.Stack with
            | [] | [_] | [_; _] | [_; _; _] -> raiseStackUnderflow state
            | (first :: second :: third :: fourth :: rest) ->
                state.withNewStack (third :: fourth :: first :: second :: rest)

        let rotWord (state : State) =
            match state.Stack with
            | [] | [_] | [_; _] -> raiseStackUnderflow state
            | (first :: second :: third :: rest) ->
                state.withNewStack (second :: third :: first :: rest)

        let pickWord (state : State) =
            match state.Stack with
            | [] -> raiseStackUnderflow state
            | (depth :: rest) ->
                match depth with
                | Parser.Integer value -> 
                    try
                        let picked = List.nth rest value
                        state.withNewStack (picked :: rest)
                    with
                    | :? System.ArgumentException as ex -> raiseStackUnderflow state
                | _ -> raise <| Errors.IntegerExpected (sprintf "Expected Integer with word 'pick', got %+A" depth)

        let notWord (state : State) =
            match state.Stack with
            | [] -> raiseStackUnderflow state
            | (top :: rest) ->
                Parser.Boolean (not <| top.IsTruthy()) :: rest
                |> state.withNewStack
                    
        type MathOperation =
            | Addition
            | Subtraction
            | Division
            | Multiplication
            | Modulo
        and ComparisonOperation =
            | LessThan
            | LessThanOrEqual
            | GreaterThan
            | GreaterThanOrEqual
            | Equal
            | NotEqual
        and BooleanOperation =
            | And
            | Or
            | Xor

        let inline doMathOperation (operation : MathOperation) left right =
            match operation with
            | Addition -> left + right
            | Subtraction ->  left - right
            | Multiplication -> left * right
            | Division -> left / right
            | Modulo -> left % right

        let inline doComparisonOperation (operation : ComparisonOperation) left right =
            match operation with
            | LessThan -> left < right
            | LessThanOrEqual -> left <= right
            | GreaterThan -> left > right
            | GreaterThanOrEqual -> left >= right
            | Equal -> left = right
            | NotEqual -> left <> right

        let inline doBooleanOperation (operation : BooleanOperation) left right =
            match operation with
            | And -> left && right
            | Or -> left || right
            | Xor -> (left && not right) || (not left && right)

        let raiseInvalidOperation left right =
            raise <| Errors.InvalidOperation (sprintf "Types '%+A' and '%+A' cannot be added together." left right)

        let booleanOperations (state : State) operation =
            match state.Stack with
            | [] | [_] -> raiseStackUnderflow state
            | (left :: right :: rest) ->
                let lvalue = left.IsTruthy()
                let rvalue = right.IsTruthy()
                Parser.Boolean (doBooleanOperation operation lvalue rvalue) :: rest
                |> state.withNewStack

        let comparisonOperation (state : State) operation =
            match state.Stack with
            | [] | [_] -> raiseStackUnderflow state
            | (left :: right :: rest) ->
                match left with
                | Parser.Integer lvalue ->
                    match right with
                    | Parser.Integer rvalue ->
                        Parser.Boolean (doComparisonOperation operation lvalue rvalue) :: rest
                        |> state.withNewStack
                    | Parser.Real rvalue ->
                        Parser.Boolean (doComparisonOperation operation (double lvalue) rvalue) :: rest
                        |> state.withNewStack
                    | _ -> raiseInvalidOperation left right

                | Parser.Real lvalue ->
                    match right with
                    | Parser.Real rvalue ->
                        Parser.Boolean (doComparisonOperation operation lvalue rvalue) :: rest
                        |> state.withNewStack
                    | Parser.Integer rvalue ->
                        Parser.Boolean (doComparisonOperation operation lvalue (double rvalue)) :: rest
                        |> state.withNewStack
                    | _ -> raiseInvalidOperation left right

                | Parser.StringLiteral lvalue ->
                    match right with
                    | Parser.StringLiteral rvalue ->
                        Parser.Boolean (doComparisonOperation operation lvalue rvalue) :: rest
                        |> state.withNewStack
                    | _ -> raiseInvalidOperation left right

                | Parser.Boolean lvalue ->
                    match right with
                    | Parser.Boolean rvalue ->
                        Parser.Boolean (doComparisonOperation operation lvalue rvalue) :: rest
                        |> state.withNewStack
                    | _ -> raiseInvalidOperation left right
                | _ -> raiseInvalidOperation left right

        let mathAndStringOperations (state : State) operation =
            match state.Stack with
            | [] | [_] -> raiseStackUnderflow state
            | (left :: right :: rest) ->
                match left with
                | Parser.Integer lvalue ->
                    match right with
                    | Parser.Integer rvalue ->
                        Parser.Integer (doMathOperation operation lvalue rvalue) :: rest
                        |> state.withNewStack
                    | Parser.Real rvalue ->
                        Parser.Real (doMathOperation operation (double lvalue) rvalue) :: rest
                        |> state.withNewStack
                    | _ -> raiseInvalidOperation left right

                | Parser.Real lvalue ->
                    match right with
                    | Parser.Real rvalue ->
                        Parser.Real (doMathOperation operation lvalue rvalue) :: rest
                        |> state.withNewStack
                    | Parser.Integer rvalue ->
                        Parser.Real (doMathOperation operation lvalue (double rvalue)) :: rest
                        |> state.withNewStack
                    | _ -> raiseInvalidOperation left right

                | Parser.StringLiteral lvalue ->
                    match right with
                    | Parser.StringLiteral rvalue ->
                        match operation with
                        | Addition ->
                            (Parser.StringLiteral (lvalue + rvalue)) :: rest
                            |> state.withNewStack
                        | _ -> raiseInvalidOperation left right
                    | _ -> raiseInvalidOperation left right
                | _ -> raiseInvalidOperation left right

    let dispatchPrimitiveWord (primitiveWord : string) (state : State) =
        match primitiveWord with
        | "stack" -> PrimitiveWords.stackWord state
        | "if" -> PrimitiveWords.ifWord state
        | "drop" -> PrimitiveWords.dropWord state
        | "swap" -> PrimitiveWords.swapWord state
        | "2swap" -> PrimitiveWords.twoSwapWord state
        | "!" -> state // set
        | "@" -> state // get
        | "rot" -> PrimitiveWords.rotWord state
        | "pick" -> PrimitiveWords.pickWord state
        | "+" -> PrimitiveWords.mathAndStringOperations state PrimitiveWords.Addition
        | "-" -> PrimitiveWords.mathAndStringOperations state PrimitiveWords.Subtraction
        | "*" -> PrimitiveWords.mathAndStringOperations state PrimitiveWords.Multiplication
        | "/" -> PrimitiveWords.mathAndStringOperations state PrimitiveWords.Division
        | "mod" -> PrimitiveWords.mathAndStringOperations state PrimitiveWords.Modulo
        | "<" -> PrimitiveWords.comparisonOperation state PrimitiveWords.LessThan
        | ">" -> PrimitiveWords.comparisonOperation state PrimitiveWords.GreaterThan
        | "<=" -> PrimitiveWords.comparisonOperation state PrimitiveWords.LessThanOrEqual
        | ">=" -> PrimitiveWords.comparisonOperation state PrimitiveWords.GreaterThanOrEqual
        | "=" -> PrimitiveWords.comparisonOperation state PrimitiveWords.Equal
        | "<>" -> PrimitiveWords.comparisonOperation state PrimitiveWords.NotEqual
        | "not" -> PrimitiveWords.notWord state
        | "and" -> PrimitiveWords.booleanOperations state PrimitiveWords.And
        | "or" -> PrimitiveWords.booleanOperations state PrimitiveWords.Or
        | "xor" -> PrimitiveWords.booleanOperations state PrimitiveWords.Xor
        | _ -> raise <| Errors.InvalidOperation (sprintf "Primitive word expected, got '%+A'." primitiveWord)

    let dispatchWord (state : State) =
        match state.ExecutionStream with
        | [] ->
            let (symTable, stack, stream) = getInterpreterState state
            raise <| Errors.MissingParseNodeInExecutionStream(symTable, stack, stream)
        | ((node_ : Parser.Node) :: nodes_) ->
            match node_ with
            | Parser.Word name ->
                match Map.tryFind name state.SymbolTable with
                | Some wordDef -> 
                    { ExecutionStream = wordDef @ state.ExecutionStream;
                        Stack = state.Stack;
                        SymbolTable = state.SymbolTable }
                | None -> 
                    dispatchPrimitiveWord name state
            | _ -> raise <| Errors.UnexpectedParserNode (sprintf "%+A" node_)

    let pushData (x : Parser.Node) (state : State) =
        state.withNewStack <| x :: state.Stack

    let executeDirective (x : Parser.Node) (state : State) =
        state

    let interpret (state : State) =
        let rec interpretHelper (state_ : State) =
            match state_.ExecutionStream with
            | [] -> state_
            | (x :: xs) ->
                match x with
                | Parser.Word _ ->
                    let updatedState = dispatchWord state_
                    interpretHelper { SymbolTable = updatedState.SymbolTable;
                        ExecutionStream = xs; Stack = updatedState.Stack } 
                | Parser.Real _ | Parser.Integer _ | Parser.Boolean _
                | Parser.StringLiteral _ | Parser.Quotation _ ->
                    let updatedState = pushData x state_
                    interpretHelper { SymbolTable = updatedState.SymbolTable;
                        ExecutionStream = xs; Stack = updatedState.Stack }
                | Parser.Directive _ ->
                    let updatedState = executeDirective x state_
                    interpretHelper { SymbolTable = updatedState.SymbolTable;
                        ExecutionStream = xs; Stack = updatedState.Stack }
        interpretHelper state