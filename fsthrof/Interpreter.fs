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

        member this.withNewExecutionStream(executionStream) =
            { SymbolTable = this.SymbolTable; ExecutionStream = executionStream;
                Stack = this.Stack }

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

    let applyQuotationToStack (q : list<Parser.Node>) (state : State) =
        state.withNewStack <| (List.rev q) @ state.Stack

    let raiseStackUnderflow (state : State) =
        let (_, stack, stream) = getInterpreterState state
        raise <| Errors.StackUnderflow(stack, stream)

    module PrimitiveWords =

        let ifWord (state : State) = 
            match state.Stack with
            | [] | [_ ; _] | [_] -> raiseStackUnderflow state
            | (falseQuotation :: trueQuotation :: predicate :: stackRest) ->
                let newState = state.withNewStack stackRest
                match predicate with
                | Parser.Real 0.0 | Parser.Integer 0 | Parser.Boolean false ->
                    match falseQuotation with
                    | Parser.Quotation q ->
                        applyQuotationToStack q newState
                    | _ -> raise <| Errors.UnexpectedParserNode (sprintf "'if' must be provided quotations for true & false conditions, got: %+A" falseQuotation)
                | Parser.Word _ | Parser.Quotation _ | Parser.StringLiteral _ | Parser.Integer _ | Parser.Real _ | Parser.Boolean _ ->
                    match trueQuotation with
                    | Parser.Quotation q -> applyQuotationToStack q newState
                    | _ -> raise <| Errors.UnexpectedParserNode (sprintf "'if' must be provided quotations for true & false conditions, got: %+A" trueQuotation)
                | _ -> raise <| Errors.UnexpectedParserNode (sprintf "%+A" predicate)

        let  dropWord (state : State) =
            match state.Stack with
            | [] -> raiseStackUnderflow state
            | _ -> state.withNewStack state.Stack.Tail

        let stackWord (state : State) =
            let rec printType (elem : Parser.Node) =
                match elem with
                | Parser.Integer value -> printf "%d" value
                | Parser.Real value -> printf "%f" value
                | Parser.Boolean value -> printf "%b" value
                | Parser.StringLiteral value -> printf "\"%s\"" value
                | Parser.Word value -> printf "%s" value
                | Parser.Quotation value ->
                    printf "[ "
                    for x in value do
                        printType x
                        printf " "
                    printf "]"
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
            | (right :: left :: rest) ->
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
            | (right :: left :: rest) ->
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
        | ((node : Parser.Node) :: nodes) ->
            match node with
            | Parser.Word name ->
                match Map.tryFind name state.SymbolTable with
                | Some wordDef -> 
                    state.withNewExecutionStream <| wordDef @ nodes
                | None -> 
                    dispatchPrimitiveWord name (state.withNewExecutionStream nodes)
            | _ -> raise <| Errors.UnexpectedParserNode (sprintf "%+A" node)

    let pushData (x : Parser.Node) (state : State) =
        state.withNewStack <| x :: state.Stack

    let popData (state : State) =
        match state.Stack with
        | (top :: rest) -> (top, state.withNewStack <| rest)
        | _ -> raiseStackUnderflow state

    let executeDirective (x : Parser.Node) (state : State) =
        match x with
        | Parser.Directive Parser.LoadFile ->
            let (fileNameNode, newState) = popData state
            match fileNameNode with
            | Parser.StringLiteral fileName ->
                let parsedContent = Parser.parse <| Tokenizer.tokenize(fileName)
                {
                    SymbolTable = Map.join <| newState.SymbolTable <| parsedContent.SymbolTable;
                    ExecutionStream = parsedContent.ExecutionStream @ newState.ExecutionStream;
                    Stack = newState.Stack
                }
            | _ -> raise <| Errors.UnexpectedParserNode (sprintf "Expecting StringLiteral, got %+A" fileNameNode)
        | _ -> raise <| Errors.UnexpectedDirective (sprintf "%+A" x)

    let interpret (state : State) =
        let rec interpretHelper (state : State) =
            match state.ExecutionStream with
            | [] -> state
            | (x :: xs) ->
                match x with
                | Parser.Word _ ->
                    let updatedState = dispatchWord state
                    interpretHelper updatedState
                | Parser.Real _ | Parser.Integer _ | Parser.Boolean _
                | Parser.StringLiteral _ | Parser.Quotation _ ->
                    let updatedState = pushData x state
                    interpretHelper <| updatedState.withNewExecutionStream xs
                | Parser.Directive _ ->
                    interpretHelper <| executeDirective x (state.withNewExecutionStream xs)
        interpretHelper state

    let loadFile (state : State) (filename : string) =
        let parsedContent = Parser.parse <| Tokenizer.tokenize(filename)
        interpret { 
            SymbolTable = Map.join <| state.SymbolTable <| parsedContent.SymbolTable;
            ExecutionStream = parsedContent.ExecutionStream @ state.ExecutionStream;
            Stack = state.Stack
        }