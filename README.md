# Monitoramento de Temperatura Corporal com ESP32 e DS18B20

Essa sessão do projeto demonstra como configurar um ESP32 para monitorar a temperatura corporal de idosos usando um sensor DS18B20 e enviar os dados de temperatura via MQTT para um broker (HiveMQ).

## Requisitos

### Hardware

- ESP32
- Sensor de Temperatura DS18B20
- Resistor de 4.7kΩ

### Montagem do Hardware

![Desenho Esquemático](imagens/esquematico.jpeg)

### Software

- Arduino IDE

## Configuração do Ambiente

### Configuração do ESP32

1. **Adicionar Suporte à Placa ESP32 na Arduino IDE**:
   - Abra a Arduino IDE.
   - Vá em `Arquivo` > `Preferências`.
   - Na caixa de diálogo que se abre, localize o campo `URLs adicionais para Gerenciadores de Placas` e adicione a seguinte URL:
     ```
     https://dl.espressif.com/dl/package_esp32_index.json
     ```
   - Clique em `OK` para fechar a caixa de diálogo.

2. **Instalar a Placa ESP32**:
   - Vá em `Ferramentas` > `Placa` > `Gerenciador de Placas`.
   - Na caixa de busca, digite `esp32`.
   - Selecione `esp32` da Espressif Systems e clique em `Instalar`.
   - Após a instalação, feche o Gerenciador de Placas.

3. **Instalar as Bibliotecas Necessárias**:
   - Vá em `Sketch` > `Incluir Biblioteca` > `Gerenciar Bibliotecas`.
   - Instale as seguintes bibliotecas:
     - `OneWire` (autor: Jim Studt et al)
     - `DallasTemperature` (autor: Miles Burton et al)
     - `PubSubClient` (autor: Nick O'Leary)
     - `WiFiManager` (autor: tzapu)
     - `ArduinoJson` (autor: Benoit Blanchon)

4. **Carregar o Código**:
   - O código fonte completo para o ESP32 está disponível na pasta `esp32/esp32ds20b18` deste repositório.
   - Abra o Arduino IDE e carregue o código fonte (`esp32/esp32ds20b18`).
   - Conecte o ESP32 ao computador via cabo USB.
   - Vá em `Ferramentas` > `Placa` e selecione a placa ESP32 que você está usando (por exemplo, `ESP32 Dev Module`).
   - Certifique-se de selecionar a porta correta em `Ferramentas` > `Porta`. Se não souber qual é a porta tente retirar do USB e abrir o menu novamente para ver qual porta sumiu ou conecte novamente e veja qual porta nova apareceu.
   - Ajuste outras configurações normalmente estão feitas corretamente, mas verifique se o `Partition Scheme` está no default with SPIFFS, pois usaremos o sistema de arquivos nesse projeto.
   - Clique no botão de upload (seta para a direita) na Arduino IDE.
   - Aguarde até que o upload esteja completo e o ESP32 reinicie.

### Configuração do WiFiManager

O WiFiManager é utilizado para configurar a conexão WiFi e parâmetros MQTT (broker, porta, tópico) de forma fácil e dinâmica, sem a necessidade de reprogramar o ESP32.

1. **Iniciar o WiFiManager**:
   - Após upload do código, no primeiro boot, ou se a conexão WiFi não puder ser estabelecida, o ESP32 cria um ponto de acesso (AP).
   - Conecte-se ao AP usando um dispositivo (computador, celular) como se você fosse entrar em um WiFi, porém o roteador se chamará `ESP32_AP`. Uma janela do navegador será aberta com um portal de configuração. Caso não se abra automaticamente entre num navegador e acesse esse IP: `192.168.4.1`. Se ainda assim não entrar verifique se os seus dados móveis estão desativados.

2. **Configurar WiFi e MQTT**:
   - Após se conectar ao AP, o navegador redireciona automaticamente para a página de configuração do WiFiManager.
   - Selecione a rede WiFi desejada, insira a senha e preencha os campos personalizados para o broker MQTT, porta e tópico.
   - Salve as configurações. O ESP32 reiniciará e tentará se conectar à rede WiFi e ao broker MQTT com as configurações fornecidas.

**Agora é só verificar se as informações estão chegando no tópico com a próxima parte do projeto!**

OBS: Caso precise reiniciar a placa em modo AP basta sair da região do wifi e ligá-la ou colocar um objeto metálico entre o pino 19 e GND por 3 segundos.
___

# Telegram Bot com Node-RED e InfluxDB

Essa sessão do projeto demonstra como configurar um bot do Telegram utilizando Node-RED para monitorar a temperatura corporal de idosos. O bot gerencia pessoas que irão receber as mensagens e armazenar dados em um banco de dados InfluxDB.

## Pré-requisitos

- [Git (última versão)](https://www.git-scm.com/download/win)
- [Node.js (recomendado: versão 20 ou superior)](https://nodejs.org/en/download/prebuilt-installer)
- [Node-RED (recomendado: versão 3 ou superior)](https://nodered.org/docs/getting-started/local#installing-with-npm)
- Conta no Telegram para criar o bot
- Conta no InfluxDB Cloud

## Criando o Bot do Telegram

1. Abra o Telegram e procure por @BotFather.
2. Inicie uma conversa com o BotFather e envie `/start` seguido de `/newbot`.
3. Siga as instruções para criar um novo bot. Você receberá um token de API. Salve este token.

## Configurando o InfluxDB

1. Crie uma conta no [InfluxDB Cloud](https://cloud2.influxdata.com/signup).
2. Crie um bucket onde você armazenará os dados.
3. Gere um token de autenticação com permissões para acessar e escrever no bucket.
4. Anote a URL do InfluxDB, o nome do bucket, o nome da organização e o token gerado.

## Configurando o Node-RED

### Importando o Fluxo

1. Clone o repositório do GitHub:
2. Dentro do próprio Git abra o node-red digitando o comando para rodar o Node-RED:
```sh
node-red
```
3. Abra o Node-RED em um navegador web acessando http://localhost:1880.
5. Antes de importar o fluxo é importante você instalar as seguintes dependências: `node-red-contrib-telegrambot` e `node-red-contrib-influxdb`. Para instalar esses pacotes, vá para o menu no canto superior direito do Node-RED, selecione "Manage palette", e instale cada um deles.
4. No Node-RED, clique no menu no canto superior direito, selecione "Import", e importe o arquivo JSON localizado em `nodered/telegrambot.json` do repositório clonado.

### Configurações Importantes
1. Substitua your-influxdb-cloud-url (exemplo: https://us-east-1-1.aws.cloud2.influxdata.com), your-org, your-bucket e your-influxdb-token pelas informações do seu InfluxDB Cloud.
2. Configure o nó telegram bot com o nome e o token do seu bot do Telegram.
3. Substitua no nó mqtt in o your/topic com o tópico MQTT onde as temperaturas serão publicadas.

## Regra de negócio das Mensagens de Temperatura
O fluxo MQTT receberá a temperatura corporal e a função `Processar a temperatura` verificará se a temperatura está acima de 37.5 ou abaixo de 35.5, enviando notificações para os chat IDs registrados pelo comando `/cadastrar` no bot. Caso queira altera a regra, modifique esse nó.

## Usando os Comandos do Bot do Telegram

Após configurar e iniciar o bot do Telegram, você pode interagir com ele usando os seguintes comandos:

### /cadastrar

Comando para se cadastrar e receber notificações de temperatura.

**Uso:**
1. Abra o Telegram e inicie uma conversa com o bot.
2. Envie o comando `/cadastrar`.

**Resposta:**
- Se o chat ID não estiver cadastrado, você receberá a mensagem: "Você foi cadastrado com sucesso!"
- Se o chat ID já estiver cadastrado, você receberá a mensagem: "Você já está cadastrado."

### /descadastrar

Comando para se descadastrar e parar de receber notificações de temperatura.

**Uso:**
1. Abra o Telegram e inicie uma conversa com o bot.
2. Envie o comando `/descadastrar`.

**Resposta:**
- Se o chat ID estiver cadastrado, você receberá a mensagem: "Você foi descadastrado com sucesso!"
- Se o chat ID não estiver cadastrado, você receberá a mensagem: "Você não está cadastrado."

### /temperatura

Comando para verificar a última temperatura lida.

**Uso:**
1. Abra o Telegram e inicie uma conversa com o bot.
2. Envie o comando `/temperatura`.

**Resposta:**
- Se o chat ID estiver cadastrado, e existir uma última temperatura lida você receberá a mensagem: "A última temperatura lida foi de: [valor]"
- Se o chat ID estiver cadastrado, e não existir uma última temperatura lida você receberá a mensagem: "Ainda não há registro de última temperatura lida"
- Se o chat ID não estiver cadastrado, você receberá a mensagem: "Você não está cadastrado!"

## Notificações de Temperatura

O bot enviará notificações automáticas para os usuários cadastrados com base nas leituras de temperatura recebidas via MQTT.

**Condições:**
- Se a temperatura estiver acima de 37.5°C, o bot enviará a mensagem: "Atenção! A temperatura está elevada: [valor]"
- Se a temperatura estiver abaixo de 35.5°C, o bot enviará a mensagem: "Atenção! A temperatura está baixa: [valor]"

___

# Configurando o Grafana

1. **Adicionar a Fonte de Dados do InfluxDB:**
   - Faça login no Grafana e vá para **Configuration** > **Data Sources**.
   - Clique em **Add data source** e selecione **InfluxDB**.
   - Configure a fonte de dados com as seguintes informações:
     - **URL:** `https://us-west-2-1.aws.cloud2.influxdata.com` (ou outra URL do seu InfluxDB Cloud)
     - **Organization:** Seu nome de organização no InfluxDB Cloud
     - **Token:** Crie um token para o grafana assim como fez como o nodered e cole aqui
     - **Default Bucket:** O nome do bucket que você criou

2. **Criar um Novo Painel:**
   - No Grafana, vá para o dashboard onde deseja adicionar a tabela.
   - Clique em **Add panel** e selecione **Table**.

3. **Configurar a Consulta Flux:**
   - No editor de consultas, selecione **Flux** como o tipo de consulta.
   - Insira a consulta Flux para buscar os dados desejados. Exemplo de consulta:
     ```flux
     from(bucket: "nome_do_seu_bucket")
       |> range(start: v.timeRangeStart, stop:v.timeRangeStop)
       |> filter(fn: (r) =>
        r._measurement == "nome_da_sua_medida" and
        r._field == "nome_do_seu_campo"
       )
     ```

4. **Outras configurações:**
   - Ajuste formatação do gráfico e nome do dashboard conforme necessário.

5. **Salvando e abrindo quando necessário**
   -  Após todos ajustes basta você clicar em salvar. E após isso basta entrar dentro dos seus dashboards do Grafana e seu dashboard estará lá para uso.