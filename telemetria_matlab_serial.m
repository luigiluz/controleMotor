clear
clc
 
%% definições do usuário sobre portas e indicadores do gráfico

serialPort = 'COM7';            % porta serial a ser utilizada
plotTitle = 'Dados transmitidos pela porta serial';  % título do gráfico
xLabel = 'Instante de tempo';           % x-axis label
yLabel = 'Velocidade (rpm)';                % y-axis label
plotGrid = 'on';                % ativar as gratículas do gráfico
min = 0;                     % valor mínimo do eixo y
max = 2300;                      % valor máximo do eixo y
scrollWidth = 10;               % quantidade de amostras no eixo do tempo
delay = .25;                    % make sure sample faster than resolution
 
% variaveis responsaveis pelo tempo de aquisição e também pelos dados
% adquiridos
time = 0;
data = 0;
count = 0;
 
% configurações da janela do gráfico
plotGraph = plot(time,data,'LineWidth',2);
title(plotTitle,'FontSize',15);
xlabel(xLabel,'FontSize',10);
ylabel(yLabel,'FontSize',10);
axis([0 10 min max]);
grid(plotGrid);
 
% estabelecendo a comunicação serial
s = serial(serialPort); %tambem podemos definir o baud rate aqui
disp('Close Plot to End Session');
% "abrindo" o arduino como se fosse um arquivo para permitir a obtenção dos
% dados
fopen(s);
% salva o tempo atual, para posteriormente o toc dizer o tempo medido entre
% os dois
tic

% enquanto o plot estiver ativo, ou seja, a janela estiver aberta
while ishandle(plotGraph) 
     
    % faremos a leitura da porta serial como um float
    dat = fscanf(s,'%f');
    % isempty só é verdade para um vetor vazio
    % entao, ~isempty(dat) sempre será verdade enquanto data for diferente
    % do vetor vazio
    % isfloat só é verdade se o parâmetro é um float
    if(~isempty(dat) && isfloat(dat)) % conferimos se dat possui valor e se é um float        
        % incrementados o contador que servirá como o indice para acessar
        % os vetores de tempo e de data
        count = count + 1;    
        time(count) = toc;    % pegamos o tempo entre as ocorrências do tic e toc e atribuimos ao vetor tempo
        data(count) = dat(1); % pegamos o valor transmitido e atribuimos para o vetor data         
         
        %definindo o eixo do tempo conforme o período para se o ter o
        %"scroll"
        %se o scrollWidth for maior que zero
        if(scrollWidth > 0)
        set(plotGraph,'XData',time(time > time(count)-scrollWidth),'YData',data(time > time(count)-scrollWidth));
        axis([time(count)-scrollWidth time(count) min max]);
        else
        set(plotGraph,'XData',time,'YData',data);
        axis([0 time(count) min max]);
        end
         
        % pausa para que seja possível atualizar o gráfico
        pause(delay);
    end
end
 
% "fechamos" o arduino
fclose(s);
% limpamos as variaveis que foram utilizadas para processamento
clear count dat delay max min plotGraph plotGrid plotTitle s ...
        scrollWidth serialPort xLabel yLabel;
disp('Session Terminated...');