#include "http_server.h"

#include "config.h"
#include "control.h"
#include "cJSON.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "HTTP";
static httpd_handle_t server;

static const char INDEX_HTML[] =
"<!doctype html>\n"
"<html lang=\"es\">\n"
"<head>\n"
"  <meta charset=\"utf-8\">\n"
"  <meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n"
"  <title>RPM Control</title>\n"
"  <link rel=\"icon\" href=\"/icon.svg\" type=\"image/svg+xml\">\n"
"  <link rel=\"stylesheet\" href=\"/styles.css\">\n"
"</head>\n"
"<body>\n"
"  <main class=\"shell\">\n"
"    <section class=\"hero\">\n"
"      <div class=\"brand\">\n"
"        <img src=\"/icon.svg\" alt=\"\" class=\"brand-icon\">\n"
"        <div>\n"
"          <p class=\"eyebrow\">ESP32-C6 PID</p>\n"
"          <h1>RPM Control Studio</h1>\n"
"        </div>\n"
"      </div>\n"
"      <div class=\"live-badge\" id=\"connectionState\">Conectando</div>\n"
"    </section>\n"
"    <section class=\"grid\">\n"
"      <article class=\"panel gauge-panel\">\n"
"        <div class=\"panel-title\">\n"
"          <span>RPM actual</span>\n"
"          <strong id=\"rpmValue\">--</strong>\n"
"        </div>\n"
"        <div class=\"gauge\">\n"
"          <div class=\"gauge-ring\"><div class=\"needle\" id=\"needle\"></div></div>\n"
"          <div class=\"gauge-center\"><span id=\"rpmMini\">0</span><small>rpm</small></div>\n"
"        </div>\n"
"        <div class=\"metric-row\">\n"
"          <div><span>Referencia</span><b id=\"setpointValue\">--</b></div>\n"
"          <div><span>Duty</span><b id=\"dutyValue\">--</b></div>\n"
"          <div><span>Salida</span><b id=\"outputValue\">--</b></div>\n"
"        </div>\n"
"      </article>\n"
"      <article class=\"panel command-panel\">\n"
"        <div class=\"panel-title compact\"><span>Nueva referencia</span></div>\n"
"        <form id=\"setpointForm\" class=\"setpoint-form\">\n"
"          <label for=\"setpointInput\">RPM objetivo</label>\n"
"          <div class=\"input-line\">\n"
"            <input id=\"setpointInput\" name=\"rpm\" type=\"number\" min=\"0\" max=\"9000\" step=\"1\" inputmode=\"numeric\" required>\n"
"            <button type=\"submit\">Aplicar</button>\n"
"          </div>\n"
"          <p id=\"formMessage\" class=\"form-message\">Rango permitido: 0 a 9000 RPM</p>\n"
"        </form>\n"
"        <div class=\"icon-card\">\n"
"          <img src=\"/icon.svg\" alt=\"Icono del panel\">\n"
"          <div><span>Apartado icon</span><b>Identidad visual embebida</b></div>\n"
"        </div>\n"
"      </article>\n"
"      <article class=\"panel history-panel\">\n"
"        <div class=\"panel-title compact\"><span>Historico RPM</span><b id=\"samplesValue\">0 muestras</b></div>\n"
"        <canvas id=\"historyChart\" width=\"900\" height=\"320\"></canvas>\n"
"      </article>\n"
"    </section>\n"
"  </main>\n"
"  <script src=\"/app.js\"></script>\n"
"</body>\n"
"</html>\n";

static const char STYLES_CSS[] =
":root{color-scheme:dark;--bg:#080a0f;--panel:#111722;--panel2:#151d2b;--text:#eff6ff;--muted:#8ea0b7;--line:#263244;--cyan:#32d7ff;--green:#66f2a5;--yellow:#f9c74f;--red:#ff6b6b}*{box-sizing:border-box}body{margin:0;min-height:100vh;background:radial-gradient(circle at 20% 0%,#19324a 0,#080a0f 34%),linear-gradient(135deg,#080a0f,#111318);font-family:Inter,Segoe UI,Arial,sans-serif;color:var(--text)}.shell{width:min(1180px,calc(100% - 28px));margin:0 auto;padding:24px 0}.hero{display:flex;align-items:center;justify-content:space-between;gap:16px;min-height:118px}.brand{display:flex;align-items:center;gap:16px}.brand-icon{width:64px;height:64px}.eyebrow{margin:0 0 5px;color:var(--green);font-size:12px;font-weight:800;letter-spacing:0;text-transform:uppercase}h1{margin:0;font-size:clamp(32px,5vw,64px);line-height:.95;letter-spacing:0}.live-badge{border:1px solid var(--line);background:#0f1724;padding:10px 14px;border-radius:8px;color:var(--yellow);font-weight:800}.live-badge.ok{color:var(--green)}.live-badge.fail{color:var(--red)}.grid{display:grid;grid-template-columns:1.15fr .85fr;gap:16px}.panel{background:linear-gradient(180deg,rgba(255,255,255,.045),rgba(255,255,255,.02));border:1px solid var(--line);border-radius:8px;padding:18px;box-shadow:0 24px 80px rgba(0,0,0,.28)}.history-panel{grid-column:1/-1}.panel-title{display:flex;align-items:flex-start;justify-content:space-between;gap:16px;color:var(--muted);font-weight:800}.panel-title strong{font-size:clamp(48px,10vw,96px);line-height:.8;color:var(--text)}.panel-title.compact b{color:var(--text)}.gauge{position:relative;height:300px;display:grid;place-items:center}.gauge-ring{position:relative;width:min(74vw,290px);aspect-ratio:1;border-radius:50%;background:conic-gradient(from 225deg,var(--cyan) 0deg,var(--green) 90deg,var(--yellow) 170deg,#263244 171deg 360deg);mask:radial-gradient(circle,transparent 0 56%,#000 57%);-webkit-mask:radial-gradient(circle,transparent 0 56%,#000 57%)}.needle{position:absolute;left:50%;top:50%;width:4px;height:42%;background:var(--text);border-radius:4px;transform-origin:50% 100%;transform:translate(-50%,-100%) rotate(-115deg);box-shadow:0 0 18px rgba(50,215,255,.55)}.gauge-center{position:absolute;display:grid;place-items:center}.gauge-center span{font-size:46px;font-weight:900}.gauge-center small{color:var(--muted);font-weight:800}.metric-row{display:grid;grid-template-columns:repeat(3,1fr);gap:10px}.metric-row div{background:var(--panel2);border:1px solid var(--line);border-radius:8px;padding:12px}.metric-row span,.setpoint-form label,.icon-card span{display:block;color:var(--muted);font-size:12px;font-weight:800;text-transform:uppercase}.metric-row b,.icon-card b{display:block;margin-top:4px;font-size:22px}.setpoint-form{display:grid;gap:10px}.input-line{display:grid;grid-template-columns:1fr auto;gap:10px}input{width:100%;height:54px;border:1px solid var(--line);background:#0d1320;color:var(--text);border-radius:8px;padding:0 14px;font-size:24px;font-weight:900}button{height:54px;border:0;border-radius:8px;padding:0 18px;background:linear-gradient(135deg,var(--cyan),var(--green));color:#061018;font-weight:950;cursor:pointer}button:active{transform:translateY(1px)}.form-message{min-height:22px;margin:0;color:var(--muted);font-weight:700}.form-message.ok{color:var(--green)}.form-message.fail{color:var(--red)}.icon-card{display:flex;align-items:center;gap:14px;margin-top:18px;padding:14px;border:1px solid var(--line);border-radius:8px;background:#0d1320}.icon-card img{width:54px;height:54px}canvas{width:100%;height:320px;display:block;background:#0b111b;border:1px solid var(--line);border-radius:8px;margin-top:14px}@media(max-width:760px){.hero{align-items:flex-start;flex-direction:column}.grid{grid-template-columns:1fr}.input-line{grid-template-columns:1fr}.metric-row{grid-template-columns:1fr}.panel-title{align-items:flex-start;flex-direction:column}.gauge{height:240px}}\n";

static const char APP_JS[] =
"const state={history:[],maxSamples:120,min:0,max:9000};\n"
"const $=id=>document.getElementById(id);\n"
"const fmt=n=>Number.isFinite(n)?Math.round(n).toLocaleString('es-CO'):'--';\n"
"function setConn(ok,text){const el=$('connectionState');el.textContent=text;el.className='live-badge '+(ok?'ok':'fail');}\n"
"function draw(){const c=$('historyChart'),ctx=c.getContext('2d'),w=c.width,h=c.height;ctx.clearRect(0,0,w,h);ctx.fillStyle='#0b111b';ctx.fillRect(0,0,w,h);ctx.strokeStyle='#263244';ctx.lineWidth=1;for(let i=1;i<5;i++){let y=h*i/5;ctx.beginPath();ctx.moveTo(0,y);ctx.lineTo(w,y);ctx.stroke();}if(state.history.length<2)return;const max=Math.max(state.max,...state.history.map(p=>p.rpm),...state.history.map(p=>p.setpoint));const x=i=>i*(w/(state.maxSamples-1));const y=v=>h-(v/max)*(h-20)-10;ctx.lineWidth=3;ctx.strokeStyle='#32d7ff';ctx.beginPath();state.history.forEach((p,i)=>{const xx=x(i),yy=y(p.rpm);i?ctx.lineTo(xx,yy):ctx.moveTo(xx,yy)});ctx.stroke();ctx.setLineDash([8,8]);ctx.strokeStyle='#66f2a5';ctx.beginPath();state.history.forEach((p,i)=>{const xx=x(i),yy=y(p.setpoint);i?ctx.lineTo(xx,yy):ctx.moveTo(xx,yy)});ctx.stroke();ctx.setLineDash([]);}\n"
"function render(data){state.min=data.min_setpoint;state.max=data.max_setpoint;$('rpmValue').textContent=fmt(data.rpm);$('rpmMini').textContent=fmt(data.rpm);$('setpointValue').textContent=fmt(data.setpoint);$('dutyValue').textContent=fmt(data.duty);$('outputValue').textContent=fmt(data.output);$('setpointInput').min=data.min_setpoint;$('setpointInput').max=data.max_setpoint;$('formMessage').textContent=`Rango permitido: ${fmt(data.min_setpoint)} a ${fmt(data.max_setpoint)} RPM`;const ratio=Math.max(0,Math.min(1,data.rpm/Math.max(data.max_setpoint,1)));$('needle').style.transform=`translate(-50%,-100%) rotate(${-115+ratio*230}deg)`;state.history.push({rpm:data.rpm,setpoint:data.setpoint});if(state.history.length>state.maxSamples)state.history.shift();$('samplesValue').textContent=`${state.history.length} muestras`;draw();}\n"
"async function poll(){try{const r=await fetch('/api/status',{cache:'no-store'});if(!r.ok)throw new Error('HTTP '+r.status);render(await r.json());setConn(true,'En linea');}catch(e){setConn(false,'Sin conexion');}}\n"
"$('setpointForm').addEventListener('submit',async ev=>{ev.preventDefault();const rpm=Number($('setpointInput').value);const msg=$('formMessage');msg.className='form-message';if(!Number.isFinite(rpm)||rpm<state.min||rpm>state.max){msg.textContent='Valor fuera de rango';msg.className='form-message fail';return;}try{const r=await fetch('/api/setpoint',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({rpm})});const data=await r.json();if(!r.ok)throw new Error(data.error||'No aplicado');msg.textContent='Referencia aplicada';msg.className='form-message ok';render(data);}catch(e){msg.textContent=e.message;msg.className='form-message fail';}});\n"
"poll();setInterval(poll,1000);\n";

static const char ICON_SVG[] =
"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 128 128\"><defs><linearGradient id=\"g\" x1=\"18\" y1=\"108\" x2=\"110\" y2=\"20\" gradientUnits=\"userSpaceOnUse\"><stop stop-color=\"#32d7ff\"/><stop offset=\"1\" stop-color=\"#66f2a5\"/></linearGradient></defs><rect width=\"128\" height=\"128\" rx=\"24\" fill=\"#080a0f\"/><path d=\"M25 82a43 43 0 1 1 78 0\" fill=\"none\" stroke=\"url(#g)\" stroke-width=\"12\" stroke-linecap=\"round\"/><path d=\"M64 69 92 39\" stroke=\"#eff6ff\" stroke-width=\"8\" stroke-linecap=\"round\"/><circle cx=\"64\" cy=\"72\" r=\"10\" fill=\"#eff6ff\"/><path d=\"M32 94h64\" stroke=\"#263244\" stroke-width=\"8\" stroke-linecap=\"round\"/></svg>\n";

static esp_err_t send_text(httpd_req_t *req, const char *type, const char *body) {
    httpd_resp_set_type(req, type);
    return httpd_resp_send(req, body, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t index_get_handler(httpd_req_t *req) {
    return send_text(req, "text/html; charset=utf-8", INDEX_HTML);
}

static esp_err_t css_get_handler(httpd_req_t *req) {
    return send_text(req, "text/css; charset=utf-8", STYLES_CSS);
}

static esp_err_t js_get_handler(httpd_req_t *req) {
    return send_text(req, "application/javascript; charset=utf-8", APP_JS);
}

static esp_err_t icon_get_handler(httpd_req_t *req) {
    return send_text(req, "image/svg+xml", ICON_SVG);
}

static esp_err_t health_get_handler(httpd_req_t *req) {
    return send_text(req, "application/json", "{\"status\":\"ok\"}");
}

static esp_err_t send_status(httpd_req_t *req) {
    control_status_t status;
    char body[192];

    control_get_status(&status);
    snprintf(body, sizeof(body),
             "{\"rpm\":%.2f,\"setpoint\":%.2f,\"output\":%.2f,\"duty\":%.2f,"
             "\"min_setpoint\":%.2f,\"max_setpoint\":%.2f}",
             status.rpm, status.setpoint, status.output, status.duty, status.min_setpoint,
             status.max_setpoint);

    return send_text(req, "application/json", body);
}

static esp_err_t status_get_handler(httpd_req_t *req) {
    return send_status(req);
}

static esp_err_t setpoint_get_handler(httpd_req_t *req) {
    control_status_t status;
    char body[96];

    control_get_status(&status);
    snprintf(body, sizeof(body),
             "{\"setpoint\":%.2f,\"min_setpoint\":%.2f,\"max_setpoint\":%.2f}",
             status.setpoint, status.min_setpoint, status.max_setpoint);

    return send_text(req, "application/json", body);
}

static esp_err_t bad_request(httpd_req_t *req, const char *message) {
    char body[96];
    httpd_resp_set_status(req, "400 Bad Request");
    snprintf(body, sizeof(body), "{\"error\":\"%s\"}", message);
    return send_text(req, "application/json", body);
}

static esp_err_t setpoint_post_handler(httpd_req_t *req) {
    if (req->content_len == 0 || req->content_len >= 128) {
        return bad_request(req, "cuerpo invalido");
    }

    char body[128];
    size_t received = 0;
    while (received < req->content_len) {
        int chunk = httpd_req_recv(req, body + received, req->content_len - received);
        if (chunk <= 0) {
            if (chunk == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            return bad_request(req, "no se pudo leer el cuerpo");
        }
        received += (size_t)chunk;
    }
    body[received] = '\0';

    cJSON *root = cJSON_Parse(body);
    if (root == NULL) {
        return bad_request(req, "json invalido");
    }

    cJSON *rpm_item = cJSON_GetObjectItemCaseSensitive(root, "rpm");
    if (!cJSON_IsNumber(rpm_item)) {
        cJSON_Delete(root);
        return bad_request(req, "rpm requerido");
    }

    float rpm = (float)rpm_item->valuedouble;
    cJSON_Delete(root);

    if (rpm < MIN_RPM_SETPOINT || rpm > MAX_RPM_SETPOINT) {
        return bad_request(req, "rpm fuera de rango");
    }

    control_set_setpoint(rpm);
    ESP_LOGI(TAG, "Referencia HTTP actualizada: %.0f RPM", rpm);
    return send_status(req);
}

esp_err_t rpm_http_server_start(void) {
    if (server != NULL) {
        return ESP_OK;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.uri_match_fn = httpd_uri_match_wildcard;

    esp_err_t err = httpd_start(&server, &config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "No se pudo iniciar HTTP: %s", esp_err_to_name(err));
        return err;
    }

    const httpd_uri_t routes[] = {
        {.uri = "/", .method = HTTP_GET, .handler = index_get_handler},
        {.uri = "/styles.css", .method = HTTP_GET, .handler = css_get_handler},
        {.uri = "/app.js", .method = HTTP_GET, .handler = js_get_handler},
        {.uri = "/icon.svg", .method = HTTP_GET, .handler = icon_get_handler},
        {.uri = "/favicon.ico", .method = HTTP_GET, .handler = icon_get_handler},
        {.uri = "/health", .method = HTTP_GET, .handler = health_get_handler},
        {.uri = "/api/status", .method = HTTP_GET, .handler = status_get_handler},
        {.uri = "/api/setpoint", .method = HTTP_GET, .handler = setpoint_get_handler},
        {.uri = "/api/setpoint", .method = HTTP_POST, .handler = setpoint_post_handler},
    };

    for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); i++) {
        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &routes[i]));
    }

    ESP_LOGI(TAG, "Servidor HTTP iniciado");
    return ESP_OK;
}
