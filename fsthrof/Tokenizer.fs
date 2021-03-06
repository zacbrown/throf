﻿namespace FSThrof

open System.IO

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

    let private getTokenFromData (token : string) =
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

    let private removeStackComments (tokens : list<string>) =
        let rec removeStackCommentsHelper (xs : list<string>) (acc : list<string>) =
            match xs with
            | [] ->
                if acc = [] then raise <| Errors.CommentNotClosed "Previously opened stack comment was not properly closed."
                else List.rev acc
            | (tok:string :: toks:list<string>) ->
                if tok = "(" then removeStackCommentsHelper (List.skipWhile (fun str -> str = ")") toks) acc
                else removeStackCommentsHelper toks (tok :: acc)
        removeStackCommentsHelper tokens []

    let private collapseStringLiterals (stream : seq<string>) =
        let rec collapseStringLiteralsHelper (pred: string -> bool) (prev : list<string>) (acc : list<string>) (rest : list<string>) =
            match rest with
            | [] -> raise Errors.UnterminatedStringLiteral
            | head :: tail ->
                if pred (head) then
                    let newAcc = List.rev (head :: acc)
                    let (collapsedStringLiteral : string) = List.reduce (fun str1 str2 -> str1 + str2) newAcc
                    (List.rev (collapsedStringLiteral :: prev)) @ rest
                else collapseStringLiteralsHelper pred prev (head :: acc) tail
        collapseStringLiteralsHelper (fun str -> str.EndsWith("\"")) [] [] (List.ofSeq stream)

    let tokenize (fileToTokenize : string) =
        let fileStream = new StreamReader(fileToTokenize)
        fileStream.ReadToEnd().Split('\n', '\r')
        |> Seq.filter (fun str -> str.StartsWith("#") = false)
        |> collapseStringLiterals
        |> Seq.collect (fun str -> str.Split(' ', '\t'))
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