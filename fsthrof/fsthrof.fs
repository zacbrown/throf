﻿module FSThrof

open System
open System.IO

module Utility =
    let dropUntil endPattern lst =
        let rec dropUntilHelper = function
        | [] -> []
        | (x :: xs) ->
            if endPattern = x then xs
            else dropUntilHelper xs
        dropUntilHelper lst

    let private collectUntil endPattern lst =
        let rec collectUntilHelper lst acc =
            match lst with
            | [] -> ([], acc)
            | (x :: xs) ->
                if endPattern = x then (xs, List.rev (x :: acc))
                else collectUntilHelper xs (x :: acc)
        collectUntilHelper lst []

module Tokenizer =
    type Token =
        | Directive of directive : string
        | WordDefinition
        | DefinitionTerminator
        | WordOrData of data : string
        | StringLiteral of data : string
        | QuotationOpen
        | QuotationClose

    exception InvalidToken of string

    let getTokenFromData (token : string) =
        let listFormToken = Seq.toList token
        match listFormToken with
        | ':' :: [] -> WordDefinition
        | ';' :: [] -> DefinitionTerminator
        | prefix :: suffix -> 
            match prefix with
            | '[' -> QuotationOpen
            | ']' -> QuotationClose
            | ':' -> Directive token
            | '\"' ->
                let stringContent = new System.String (List.toArray (List.rev (List.rev suffix).Tail))
                match (List.rev suffix).Head with 
                | '\"' -> StringLiteral stringContent
                | _ -> WordOrData token
            | _ -> WordOrData token
        | _ -> raise <| InvalidToken(token)

    let removeStackComments (tokens : list<string>) =
        let rec removeStackCommentsHelper (xs : list<string>) (acc : list<string>) =
            match xs with
            | [] -> List.rev acc
            | (tok:string :: toks:list<string>) ->
                if tok = "(" then removeStackCommentsHelper (Utility.dropUntil ")" toks) acc
                else removeStackCommentsHelper toks (tok :: acc)
        removeStackCommentsHelper tokens []

    let tokenize (fileToTokenize : string) =
        let getFileStream = new StreamReader(fileToTokenize)
        getFileStream.ReadToEnd().Split('\t', ' ', '\n', '\r')
        |> Seq.filter (fun str -> str <> "")
        |> Seq.toList
        |> removeStackComments 
        |> Seq.map getTokenFromData
        |> Seq.toList

    let printTokens (toks : list<Token>) =
        for tok in toks do
            match tok with
            | WordOrData data -> printfn "token of type %A with data %s" tok data 
            | _ -> printfn "token of type %A" tok

module Parser =
    type Directive =
        | LoadFile
        | Defer
        | Variable
    and Node =
        | Directive of directive : string
        | Word of data : string
        | Integer of num : int
        | Real of num : double
        | Boolean of value : bool
        | StringLiteral of data : string
        | Quotation of list<Node>
    and ParsedContent = {
        SymbolTable : Map<string, list<Node>>;
        ExecutionStream : list<Node>;
    }

    let private collectUntil (endToken : Tokenizer.Token) (lst : list<Tokenizer.Token>) =
        let rec collectUntilHelper (lst : list<Tokenizer.Token>) acc =
            match lst with
            | [] -> ([], acc)
            | (x :: xs) ->
                if endToken = x then (xs, List.rev acc)
                else collectUntilHelper xs (x :: acc)
        collectUntilHelper lst []

    exception InvalidTokenToNodeTransformation of string
    exception UnexpectedToken of string

    let private tokenToNode (tok : Tokenizer.Token) =
        match tok with
        | Tokenizer.Directive directive -> Directive directive
        | Tokenizer.WordOrData data ->
            try
                let numVal = Int32.Parse data
                Integer numVal
            with
            | :? System.FormatException as ex ->
                try
                    let numVal = Double.Parse data
                    Real numVal
                with
                | :? System.FormatException as ex ->
                    try
                        let boolVal = Boolean.Parse data
                        Boolean boolVal
                    with
                    | :? System.FormatException as ex ->
                        Word data
        | Tokenizer.StringLiteral data -> StringLiteral data
        | _ -> raise <| InvalidTokenToNodeTransformation (sprintf "%A" tok)

    let private collectQuotation tokens =
        let (streamRemainder, quotationStream) = (collectUntil Tokenizer.QuotationClose tokens)
        let quotationNode = Quotation (List.map (fun elem -> tokenToNode elem) quotationStream)
        (streamRemainder, quotationNode)

    let rec private transformTokensToNodes acc lst =
        match lst with
        | [] -> List.rev acc
        | ((hd : Tokenizer.Token) :: tl) ->
            match hd with
            | Tokenizer.QuotationOpen ->
                let (remainingStream, quotationNode) = collectQuotation tl
                transformTokensToNodes (quotationNode :: acc) remainingStream
            | Tokenizer.StringLiteral data -> transformTokensToNodes (StringLiteral data :: acc) tl
            | Tokenizer.WordOrData _ ->
                transformTokensToNodes ((tokenToNode hd) :: acc) tl
            | _ -> raise <| UnexpectedToken (sprintf "%A" hd)

    let parse (tokens : list<Tokenizer.Token>) =
        let rec parseHelper tokens (content : ParsedContent) =
            match tokens with
            | [] -> content
            | (tok : Tokenizer.Token) :: toks ->
                match tok with
                | Tokenizer.WordDefinition ->
                    match toks.Head with
                    | Tokenizer.WordOrData wordName ->
                        let (streamRemainder, wordDef) = (collectUntil Tokenizer.DefinitionTerminator toks)
                        let newContent = {
                            SymbolTable = content.SymbolTable.Add(wordName, transformTokensToNodes [] wordDef);
                            ExecutionStream = content.ExecutionStream
                        }
                        parseHelper streamRemainder newContent
                    | _ -> raise <| UnexpectedToken (sprintf "%+A" toks.Head)
                | Tokenizer.Directive _ ->
                    // todo: Implement directives.
                    { SymbolTable = Map.empty; ExecutionStream = []}
                | Tokenizer.QuotationOpen ->
                    let (streamRemainder, quotationNode) = collectQuotation tokens
                    let newContent = {
                        SymbolTable = content.SymbolTable;
                        ExecutionStream = content.ExecutionStream @ [quotationNode]
                    }
                    parseHelper streamRemainder newContent
                | Tokenizer.WordOrData _ ->
                    let newContent = {
                        SymbolTable = content.SymbolTable;
                        ExecutionStream = content.ExecutionStream @ [tokenToNode tok]
                    }
                    parseHelper toks newContent
                | Tokenizer.StringLiteral data ->
                    let newContent = {
                        SymbolTable = content.SymbolTable;
                        ExecutionStream = content.ExecutionStream @ [StringLiteral data]
                    }
                    parseHelper toks newContent
                | _ -> raise <| UnexpectedToken (sprintf "%A" tok)
        parseHelper tokens { SymbolTable = Map.empty; ExecutionStream = []}

    let getParserState (parsedContent : ParsedContent) =
        let symTable = Map.map (fun x y -> sprintf "%+A - %+A" x y) parsedContent.SymbolTable
        let execStream = List.map (fun x -> sprintf "%+A" x) parsedContent.ExecutionStream
        (symTable, execStream)

    let printParserState (parsedContent : ParsedContent) =
        let (symTable, execStream) = getParserState parsedContent
        printfn "Parser State"
        printfn "================="
        Map.iter (fun x -> printfn "%+A - %+A" x) symTable
        List.iter (fun x -> printfn "%+A" x) execStream
        printfn "================="

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
    exception NativeWordExpected of string
    exception StackUnderflow of string * string

    let applyQuotationToStack (q : list<Parser.Node>) (state : State) =
        state.withNewStack <| (List.rev q) @ state.Stack

    module NativeWords =
        let private raiseStackUnderflow (state : State) =
            let (_, stack, stream) = getInterpreterState state
            raise <| StackUnderflow(stack, stream)

        let ifWord (state : State) = 
            match state.Stack with
            | [] | [_ ; _] | [_] -> raiseStackUnderflow state
            | (predicate :: falseQuotation :: trueQuotation :: stack) ->
                match predicate with
                | Parser.Real 0.0 | Parser.Integer 0 | Parser.Boolean false ->
                    match falseQuotation with
                    | Parser.Quotation q -> applyQuotationToStack q state
                    | _ -> raise <| UnexpectedParserNode (sprintf "'if' must be provided quotations for true & false conditions, got: %+A" falseQuotation)
                | Parser.Word _ | Parser.Quotation _ | Parser.StringLiteral _ | Parser.Integer _ | Parser.Real _ | Parser.Boolean _ ->
                    match trueQuotation with
                    | Parser.Quotation q -> applyQuotationToStack q state
                    | _ -> raise <| UnexpectedParserNode (sprintf "'if' must be provided quotations for true & false conditions, got: %+A" trueQuotation)
                | _ -> raise <| UnexpectedParserNode (sprintf "%+A" predicate)

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
                | _ -> raise <| UnexpectedParserNode (sprintf "The interpreter is in an unexpected state. The parse node '%+A' was unexpected on the stack." elem)
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
                | _ -> raise <| IntegerExpected (sprintf "Expected Integer with word 'pick', got %+A" depth)

    let dispatchNativeWord (nativeWord : string) (state : State) =
        match nativeWord with
        | "stack" -> NativeWords.stackWord state
        | "if" -> NativeWords.ifWord state
        | "drop" -> NativeWords.dropWord state
        | "swap" -> NativeWords.swapWord state
        | "2swap" -> NativeWords.twoSwapWord state
        | "!" -> state // set
        | "@" -> state // get
        | "rot" -> NativeWords.rotWord state
        | "pick" -> NativeWords.pickWord state
        | "+" -> state
        | "-" -> state
        | "*" -> state
        | "/" -> state
        | "mod" -> state
        | "<" -> state
        | ">" -> state
        | "<=" -> state
        | ">=" -> state
        | "==" -> state
        | "<>" -> state
        | "not" -> state
        | "and" -> state
        | "or" -> state
        | "xor" -> state
        | _ -> raise <| NativeWordExpected (sprintf "%+A" nativeWord)

    let dispatchWord (state : State) =
        match state.ExecutionStream with
        | [] ->
            let (symTable, stack, stream) = getInterpreterState state
            raise <| MissingParseNodeInExecutionStream(symTable, stack, stream)
        | ((node_ : Parser.Node) :: nodes_) ->
            match node_ with
            | Parser.Word name ->
                match Map.tryFind name state.SymbolTable with
                | Some wordDef -> 
                    { ExecutionStream = wordDef @ state.ExecutionStream;
                        Stack = state.Stack;
                        SymbolTable = state.SymbolTable }
                | None -> 
                    dispatchNativeWord name state
            | _ -> raise <| UnexpectedParserNode (sprintf "%+A" node_)

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

[<EntryPoint>]
let main argv =
    let tokens = Tokenizer.tokenize(argv.[0])
    let parsedContent = Parser.parse tokens
    let interpretedState = Interpreter.interpret { SymbolTable = parsedContent.SymbolTable; ExecutionStream = parsedContent.ExecutionStream; Stack = [] }
    0
