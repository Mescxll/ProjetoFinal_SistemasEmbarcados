# Projeto U7T - Sistema Inteligente de Monitoramento Simulado para Segurança de Terrenos Industriais
Simulação do Projeto Embarcado em questão utilizando a Placa BitDogLab e o Pico SDK com linguegem C para simular as funcionalidades. Este trabalho foi desenvolvido por Maria Eduarda Campos para a Capacitação Embarca Tech.
### Descrição
Essa proposta tem como objetivo ser uma solução exclusiva para vigilância das condições do solo em indústrias pesadas, prevenindo acidentes e falhas estruturais antes que ocorram. Para isso são monitorados fatores como temperatura e umidade do solo, presença de vibrações subterrâneas e compactação do terreno. 

Aplicando na vida real, a detecção de vibração seria feita por um acelerômetro MPU605, o monitoramento de temperatura e umidade do solo pelo sensor de temperatura e umidade DHT22 e a análise da resistência do solo por um penetrómetro, sensor que mede a resistência do solo à penetração. Caso algum dos fatores estivesse fora dos padrões, ou seja, em nível crítico. Seria emitido um alerta sonoro que permitiria detectar essas anomalias e realizar manutenções corretivas de forma eficiente.
### Simulação 
- Detecção de vibração - MPU6050 -> JOYSTICK X 
- Temperatura do solo - DHT22 -> LED RGB
- Umidade do solo - DHT22 -> Matriz de LEDs
- Resistência do solo - Penetrômetro -> JOYSTICK Y 
- Sirene de alerta -> BUZZER
- Monitoramento de status -> DISPLAY OLED
  
![Captura de tela 2025-02-26 192422](https://github.com/user-attachments/assets/a1e5df3a-e617-4373-994b-8f398d5ca1e5)

### Funcionalidades
- Controlar os níveis de Vibração e Compactação pela movimentação do Joysttick.
- Controlar os níveis de temperatura ao apertar o Botão A, ligando o LED RGB em diferentes intensidades.
- Controlar os níveis de umidade ao apertar o Botão B, ligando a matriz de LEDs 5x5 em diferentes figuras.
- Ativar Buzzer caso alguma váriavel entre em nível crítico.
- Monitorar os níveis por mensagens via Dsiplay.

### Instruções:
1. Clone este repositório:
   ```sh
   git clone https://github.com/Mescxll/ProjetoFinal_SistemasEmbarcados.git
   ```
2. Compile e carregue o código no Raspberry Pi Pico W utilizando o VS Code e o Pico SDK.
3. Utilize a placa BitDogLab e os seus componentes para testar o funcionamento do programa.

