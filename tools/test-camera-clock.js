const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');

const root = path.resolve(__dirname, '..');
const html = path.join(root, 'sd-card/html');
const context = vm.createContext({});
for (const name of ['readconfigparam.js', 'readconfigcommon.js']) {
    vm.runInContext(fs.readFileSync(path.join(html, name), 'utf8'), context);
}
const template = fs.readFileSync(path.join(html, 'edit_config_template.html'), 'utf8');
vm.runInContext(template.slice(template.indexOf('function EnDisableItem('),
    template.indexOf('/* hash #description')), context);
context.setEnabled = function () {};
context.firework = { launch: function (message) { assert.fail(message); } };
const elements = {
    TakeImage_CamXclkFreqMhz_enabled: { checked: false, style: {} },
    TakeImage_CamXclkFreqMhz_text: { style: {} },
    TakeImage_CamXclkFreqMhz_value1: {
        tagName: 'SELECT', style: {}, selectedIndex: 3,
        options: ['8', '10', '16', '20'].map(value => ({ value }))
    }
};
context.document = { getElementById: id => elements[id] };
const base = fs.readFileSync(path.join(root, 'sd-card/config/config.ini'), 'utf8');

function load(line) {
    context.config_gesamt = base.replace('[TakeImage]', '[TakeImage]\n' + line);
    context.getCamConfig();
    context.WriteParameter(context.param, context.category, 'TakeImage', 'CamXclkFreqMhz', true);
}

function save() {
    context.ReadParameter(context.param, 'TakeImage', 'CamXclkFreqMhz', true);
    context.WriteConfigININew();
    return context.config_split.find(line => /^;?CamXclkFreqMhz\s*=/.test(line));
}

load('');
assert.equal(elements.TakeImage_CamXclkFreqMhz_enabled.checked, false);
assert.equal(elements.TakeImage_CamXclkFreqMhz_value1.disabled, true);
assert.equal(save(), ';CamXclkFreqMhz = 20');
for (const value of ['8', '10', '16', '20']) {
    load('CamXclkFreqMhz = ' + value);
    assert.equal(elements.TakeImage_CamXclkFreqMhz_enabled.checked, true);
    assert.equal(elements.TakeImage_CamXclkFreqMhz_value1.disabled, false);
    assert.equal(elements.TakeImage_CamXclkFreqMhz_value1.options[
        elements.TakeImage_CamXclkFreqMhz_value1.selectedIndex].value, value);
    assert.equal(save(), 'CamXclkFreqMhz = ' + value);
}
load(';CamXclkFreqMhz = 8');
assert.equal(elements.TakeImage_CamXclkFreqMhz_enabled.checked, false);
assert.equal(save(), ';CamXclkFreqMhz = 8');
load('');
elements.TakeImage_CamXclkFreqMhz_enabled.checked = true;
elements.TakeImage_CamXclkFreqMhz_value1.selectedIndex = 0;
assert.equal(save(), 'CamXclkFreqMhz = 8');
elements.TakeImage_CamXclkFreqMhz_enabled.checked = false;
assert.equal(save(), ';CamXclkFreqMhz = 8');
// Reference/ROI editors also rewrite config through this shared parser.
load('CamXclkFreqMhz = 10');
context.ParseConfig();
context.WriteConfigININew();
assert.equal(context.config_split.find(line => line.startsWith('CamXclkFreqMhz')), 'CamXclkFreqMhz = 10');
console.log('Camera clock UI load/edit/reset and config preservation passed');
