$$
\begin{align}

[\text{prog}] &\to [\text{stmt}]^*
\\ [\text{stmt}] &\to 
\begin{cases}
    exit{([\text{expr}])}; \\
    let \space\text{ident} = [\text{expr}];
\end{cases}


\\ [\text{exit}] &\to exit([\text{expr}]);

\\ [\text{expr}] &\to \text{int\_lit}

\end{align}
$$
